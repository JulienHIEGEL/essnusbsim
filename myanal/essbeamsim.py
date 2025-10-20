#
# --- Support script for ESSnuSB TT line to LEnuSTORM
#

from pprint import pprint

import xobjects as xo
import xtrack as xt

from particle import Particle

import numpy as np
import pandas as pd

import scipy

import re

class DotDict(dict):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        for key, value in self.items():
            if isinstance(value, dict):
                self[key] = DotDict(value)  # Recursively convert nested dicts

    def __getattr__(self, name):
        if name in self:
            return self[name]
        raise AttributeError(f"No such attribute: {name}")

    def __setattr__(self, name, value):
        self[name] = value

    def __delattr__(self, name):
        del self[name]
class _DotDict(dict):

    __setattr__ = dict.__setitem__
    __delattr__ = dict.__delitem__

    def __getattr__(self, key):

        def typer(candidate):
            if isinstance(candidate, dict):
                return DotDict(candidate)

            if isinstance(candidate, str):  # iterable but no need to iter
                return candidate

            try:  # other iterable are processed as list
                return [typer(item) for item in candidate]
            except TypeError:
                return candidate

            return candidate

        return typer(dict.get(self, key))
    
def line_equation(a, b):
    ''' line equation passing trhough the two points A and B : y = w*x + c '''
    w = (a[1]-b[1])/(a[0]-b[0])
    c = a[1]-w*a[0]
    return w, c

def distance_point_to_line(pline1, pline2, apoint):
    ''' distance of apoint to a line passing through pline1 and pline2 '''
    _aa = pline2[1] - pline1[1]
    _bb = pline2[0] - pline1[0]
    _cc = pline2[0]*pline1[1] - pline2[1]*pline1[0]
    _dd = np.sqrt(_aa*_aa + _bb*_bb)
    return np.abs(_aa*apoint[0] - _bb*apoint[1] + _cc)/_dd

def dipole_length_from_angle(angle, arclength):
    rcurv = arclength/angle
    return rcurv * np.sin(angle)

def rotation_matrix(theta):
    
    c = np.cos(theta)
    s = np.sin(theta)
    
    return np.array([[c, -s], [s, c]])
    #return np.array([[c, s], [-s, c]])

def create_transform(t_x, t_y, theta):
    
    translation = np.array([t_x, t_y])
    rotation = rotation_matrix(theta)
    
    transform = np.eye(3, dtype=float)
    transform[:2, :2] = rotation
    transform[:2, 2] = translation
    
    return transform

def apply_transform(transform, x):
    
    x_h = np.ones((x.shape[0] + 1, x.shape[1]), dtype=x.dtype)
    x_h[:-1, :] = x
    
    x_t = np.dot(transform, x_h)
    
    return x_t[:2] / x_t[-1]

def load_lenustrom_ring(fin):
    _dfy = pd.read_parquet(fin)
    return _dfy[['s_fr','s_srv','name_fr','name_srv','Z_cm','X_cm']].copy()

def compute_corner_from_front_center(center, width, height, angle):
    """Compute bottom-left corner of a rectangle given its front-center."""
    # Convert angle to radians
    rad = np.radians(angle)
    # Offset from center to bottom-left corner
    dx = -(width / 2) * np.cos(rad) + (height / 2) * np.sin(rad)
    dy = -(width / 2) * np.sin(rad) - (height / 2) * np.cos(rad)
    # Return adjusted position
    return center[0] + dx, center[1] + dy

def compute_corner_from_center(center, width, height, rad):
    """Compute bottom-left corner of a rectangle given its center."""
    # Convert angle to radians
    # rad = np.radians(angle)
    # Offset from center to bottom-left corner
    dx = -(width / 2) * np.cos(rad) + (height / 2) * np.sin(rad)
    dy = -(width / 2) * np.sin(rad) - (height / 2) * np.cos(rad)
    # Return adjusted position
    return center[0] + dx, center[1] + dy

class TTLINE:
    bend_iron_yoke_side = 0.30   # iron width in the dipoles beyond the aperture
        
    def __init__(self, attributes=None, layout_attr=None):
        # print(f'__init__ : {attributes=}')
        for kk, vv in attributes.items():
            # print(f' seattr: {kk} = {vv}')
            setattr(self, kk, vv)
        
        for kk, vv in layout_attr.items():
            # print(f' seattr: {kk} = {vv}')
            setattr(self, kk, vv)
 
        self.l_bend_tot = self.l_bend + 2*self.l_bend_end
        self.l_quad_tot = self.l_quad + 2*self.l_quad_end
        self.l_qfl_tot = self.l_qfl + 2*self.l_qfl_end
        
        self.line_segments = [self.trg_to_d1pivot,
                              np.sqrt((self.d2_pivot[0]-self.d1_pivot[0])**2 + (self.d2_pivot[1]-self.d1_pivot[1])**2),
                              np.sqrt((self.lenustorm_inj[0]-self.d2_pivot[0])**2  + (self.lenustorm_inj[1]-self.d2_pivot[1])**2)]
        self.line_length = np.sum(self.line_segments)
        
        self.ring_orientation, self.ring_offset = line_equation([self.lenustorm_inj[1], self.lenustorm_inj[0]],
                                                                [self.trg_to_lemond,0])
                                                                 
        self.ring_orientation_deg = self.ring_orientation*180/np.pi

        self.ring_nearest_approach = distance_point_to_line([self.lenustorm_inj[1], self.lenustorm_inj[0]],
                                                            [self.trg_to_lemond, 0],
                                                            [self.l_decay_tunnel, -self.w_decay_tunnel/2])
    

        self.d1dipole = self.create_dipole(abs(self.d1_angle))
        self.min_distance_to_quad = (self.d1dipole['width'] - self.d1dipole['deflection'])/2 * np.sin(abs(self.d1_angle/2))
        self.create_cells()
        self.elements = self.create_elements()
        return
    
    def create_dipole(self, angle):
        _dipole = {}
        _dipole['angle'] = angle
        _dipole['bending_radius'] = self.l_bend / angle
        _dipole['arc'] = 2 * _dipole['bending_radius'] * np.sin(angle/2)
        _dipole['deflection'] = _dipole['arc'] * np.sin(angle/2)
        _dipole['length'] = _dipole['bending_radius'] * np.sin(angle)
        _dipole['width'] = _dipole['deflection'] + 2*self.beam_size + 2*TTLINE.bend_iron_yoke_side
        _dipole['height'] = 1.5*self.beam_size + 2*TTLINE.bend_iron_yoke_side
        _dipole['aperx'] = _dipole['deflection'] + self.beam_size
        _dipole['apery'] = 1.2*self.beam_size
        _dipole['pivot'] = _dipole['bending_radius'] * np.tan(angle/2)
        return _dipole
        
    def create_cells(self):
        cell_length = self.line_segments[1]/4
        self.cell_length_min = self.l_bend_tot/2 + 2*self.l_quad_tot + self.l_qfl_tot/2 + 3*self.l_vac

        assert cell_length > self.cell_length_min, f' -- ERROR {cell_length=} is less than mimimum allowed {self.cell_length_min}'
        self.l_vac_adjusted = self.l_vac + (cell_length - self.cell_length_min)/3
        self.cell_length = cell_length
        return
    
    def create_elements(self):
        ''' element positions - center'''
        _elements = []
        
        # -- distances from FL center to the center of the active element (MAD-X)
        _pos1 = self.l_qfl_tot/2 + 1/2*self.l_quad_tot + self.l_vac_adjusted
        _pos2 = self.l_qfl_tot/2 + 3/2*self.l_quad_tot + 2*self.l_vac_adjusted

        _aa = self.d1_angle
        _zz = self.d1dipole['bending_radius']*np.tan(abs(_aa)/2)
        
        _scumm = 0
        _elements.append({'name' :'TARGET', 'type': 'marker', 'spos' : _scumm, 'coord' : [0, 0], 'angle' : 0.0})
        _scumm = self.d1_pivot[1] - _zz
        _elements.append({'name' : 'MB1', 'type': 'bend', 'spos': _scumm + self.l_bend/2, 
                          'coord': [ 0, _scumm + self.d1dipole['length']/2], 'angle' : 0.0})
        
        _sfl1 = _scumm + self.l_bend + self.cell_length - _zz
        _bpos = self.cell_length - _pos2
        _elements.append({ 'name' :'QF1', 'type': 'quad', 'spos' : _sfl1 - _pos2, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = self.cell_length - _pos1
        _elements.append({ 'name' :'QD1', 'type': 'quad', 'spos' : _sfl1 - _pos1, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = self.cell_length
        _elements.append({ 'name' :'QFL1', 'type': 'fquad', 'spos' : _sfl1        , 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = self.cell_length + _pos1
        _elements.append({ 'name' :'QD2', 'type': 'quad', 'spos' : _sfl1 + _pos1, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = self.cell_length + _pos2
        _elements.append({ 'name' :'QF2', 'type': 'quad', 'spos' : _sfl1 + _pos2, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
  
        _sfl2 = _sfl1 + 2*self.cell_length
        _bpos = 3*self.cell_length - _pos2
        _elements.append({ 'name' :'QF3', 'type': 'quad', 'spos' : _sfl2 - _pos2, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle': _aa})
        _bpos = 3*self.cell_length - _pos1
        _elements.append({ 'name' :'QD3', 'type': 'quad', 'spos' : _sfl2 - _pos1, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = 3*self.cell_length
        _elements.append({ 'name' :'QFL2', 'type': 'fquad', 'spos' : _sfl2        , 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = 3*self.cell_length + _pos1
        _elements.append({ 'name' :'QD4', 'type': 'quad', 'spos' : _sfl2 + _pos1, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle' : _aa})
        _bpos = 3*self.cell_length + _pos2
        _elements.append({ 'name' :'QF4', 'type': 'quad', 'spos' : _sfl2 + _pos2, 
                          'coord': [self.d1_pivot[0] + _bpos*np.sin(_aa), self.d1_pivot[1] + _bpos*np.cos(_aa)], 'angle': _aa})

        _scumm = _sfl2 + self.cell_length - _zz
        _elements.append({'name' : 'MB2', 'type': 'bend', 'spos': _scumm + self.l_bend/2, 
                          'coord': [ self.d2_pivot[0], self.d2_pivot[1] + _zz - self.d1dipole['length']/2], 'angle' : 0.0}) 
            
        _scumm += self.l_bend + (self.lenustorm_inj[1]-self.d2_pivot[1]) - _zz
        _elements.append({'name' : 'INJ', 'type': 'marker', 'spos': _scumm, 
                          'coord': [ self.d2_pivot[0], self.d2_pivot[1] + (self.lenustorm_inj[1]-self.d2_pivot[1])], 'angle' : 0.0})    
        return _elements
        
    @classmethod
    def build_line_from_inj_point(cls, injpoint=(-14.5, 15), params=None, lparams=None):
        print(f' build_line_from_end_point : {injpoint=}')
        
        _x_inj, _z_inj = injpoint
        d2toinj_x, d2toinj_z = params['d2pivot_to_inj']
        trgtod1_z = params['trg_to_d1pivot']
        
        tan_theta = (_x_inj - d2toinj_x)/(_z_inj - d2toinj_z - trgtod1_z)
        
        params.update({'lenustorm_inj': injpoint,
                       'd2_pivot' : (_x_inj - d2toinj_x,_z_inj - d2toinj_z),
                       'd1_angle': np.atan(tan_theta),
                       'd1_angle_deg' : np.atan(tan_theta)*180/np.pi,
                       'd1_pivot' : (0, params['trg_to_d1pivot']),
                       'method' : f'from lenusotrm injection point coordinates ({injpoint} [m])'
                       })
        return cls(params, lparams)

    @classmethod
    def build_line_from_angle_and_displacement(cls, angle_deg=-54.0, xdisp=-12.5, params=None, lparams=None):
        print(f' build_line_from_angle_and_displacement : {angle_deg=}, {xdisp=}, {params=}')

        angle_rad = angle_deg*np.pi/180
        _d2pivot = (xdisp, params['trg_to_d1pivot']+ xdisp/np.tan(angle_rad))
        
        params.update({'d1_angle_deg': angle_deg,
                       'd1_angle' : angle_rad,
                       'd1_pivot' : (0, params['trg_to_d1pivot']),
                       'd2_pivot' : _d2pivot,
                       'lenustorm_inj' : (_d2pivot[0] + params['d2pivot_to_inj'][0],
                                          _d2pivot[1] + params['d2pivot_to_inj'][1]),
                       'method' : f'from angle ({angle_deg}[deg]) and lateral displacement ({xdisp}[m])'
                       })
        return cls(params, lparams)

    def __repr__(self):
        return "\n".join([f"{key}: {value}" for key, value in self.__dict__.items()])

    def dmpattributes(self):
        return print(json.dumps(self.__dict__, indent=4))
        
    def get_points(self):
        _xv = [0, self.d1_pivot[0], self.d2_pivot[0], self.lenustorm_inj[0]]
        _zv = [0, self.d1_pivot[1], self.d2_pivot[1], self.lenustorm_inj[1]]
        return _xv, _zv
    
    def display(self):
        for i in vars(self):
            vv = vars(self)[i]
            print (f" {i.ljust(30,'.')} {vars(self)[i]}")
            
            
            
def particle_beta_gamma(name, pmom_gev):
    _part = Particle.from_name(name) 
    _mass = _part.mass*1.0e-3 # convert from MeV to GeV
    _bgamma = pmom_gev/_mass
    _gamma = np.sqrt(_bgamma**2+1)
    _beta = _bgamma/_gamma
    return _beta, _gamma

def generate_lenustorm(mmomentum, icycle=0, verbose=False):
    ''' Generate the LEnuSTORM ring in Xsuite'''
    
    env = xt.Environment()

    MUON_MASS_EV = 105.7e6

    env.particle_ref = xt.Particles(p0c=mmomentum*1.0e9, #eV
                                    q0=1, mass0=MUON_MASS_EV)

    env.new('QFA', xt.Quadrupole, length = 0.125, k1 =  5.45022291277778148)
    env.new('QDA', xt.Quadrupole, length = 0.25, k1 = -4.56255529153320527)
    env.new('QFS', xt.Quadrupole, length = 0.125, k1 =  4.20157995314257704)
    env.new('QDS', xt.Quadrupole, length = 0.25, k1 = -4.47765024083436547)
    env.new('QDS_to_A1', xt.Quadrupole, length = 0.25, k1 = -4.50136419788634790)
    env.new('QDS_to_A2', xt.Quadrupole, length = 0.25, k1 = -4.56741388890428279)
    env.new('QFS_to_A1', xt.Quadrupole, length = 0.25, k1 = 4.82791218101006692)
    env.new('D005', xt.Drift, length = 0.05)
    env.new('D_sex', xt.Drift, length = 0.15)
    env.new('D_stoA1', xt.Drift, length = 9.94409642653986126E-001)
    env.new('DB', xt.Drift, length = 0.6)
    env['a']= 0.6
    env['b'] = np.pi/(6*2)
    env['rho'] = 'a / b'
    env.new('B', xt.Bend, length = 'a', h = '1/rho', k0 = '1/rho')

    FODOA = env.new_line(
        name = 'fodoa',
        components = ['QFA', 'D_sex', 'D005', 'B', 'D005', 'D_sex', 'QDA', 'D_sex', 'D005', 'B', 'D005', 'D_sex', 'QFA'])

    env.new('fodoa1', 'fodoa', mode='replica')
    env.new('fodoa2', 'fodoa', mode='replica')
    env.new('fodoa3', 'fodoa', mode='replica')
    env.new('fodoa4', 'fodoa', mode='replica')
    env.new('fodoa5', 'fodoa', mode='replica')
    env.new('fodoa6', 'fodoa', mode='replica')

    HALF_ARC = env.new_line(
        name = 'half-arc',
        components = ['fodoa1', 'fodoa2', 'fodoa3', 'fodoa4', 'fodoa5', 'fodoa6']
    )

    #HALF_ARC.get_table().show()

    FODOS_to_A = env.new_line(
        name = 'FODOS_to_A', 
        components = ['QFS', 'D_stoA1', 'QDS_to_A1', 'D_stoA1', 
        'QFS_to_A1', 'D_stoA1', 'QDS_to_A2', 'D_stoA1', 'QFA']
    )

    FODOS1 = env.new_line(
        name = 'fodos1', 
        components = ['QFS', 'D_sex', 'D005', 'DB', 'D005', 'D_sex', 
        'QDS', 'D_sex', 'D005', 'DB', 'D005', 'D_sex', 'QFS']
    )

    STRAIGHT = env.new_line(
        name = 'straight',
        components = ['fodos1']*20
    )

    #FODOS_to_A.get_table().show()

    HALF_RING = STRAIGHT + FODOS_to_A + HALF_ARC - FODOS_to_A + STRAIGHT

    #HALF_RING.get_table().show()

    FULL_RING = HALF_RING+HALF_RING

    '''
    straight = FODOS1+FODOS1

    half_ring: line = (straight, FODOS_to_A, arc, -FODOS_to_A, straight)
    full_ring: line = (2*half_ring)

    Optimized parameters are:
    QFA[K1] =  5.45022291277778148E+000
    QDA[K1] = -4.56255529153320527E+000
    QFS[K1] =  4.20157995314257704E+000
    QDS[K1] = -4.47765024083436547E+000
    QDS_TO_A1[K1] = -4.50136419788634790E+000
    QDS_TO_A2[K1] = -4.56741388890428279E+000
    QFS_TO_A1[K1] =  4.82791218101006692E+000
    D_STOA1[L] =  9.94409642653986126E-001
    '''
    
    # -- cycle the ring to start from the first big gap of the SS
    if icycle != 0 :
        FULL_RING = FULL_RING.cycle(index_first_element=icycle)
        
    context = xo.ContextCpu()         # For CPU
    # context = xo.ContextCupy()      # For CUDA GPUs
    # context = xo.ContextPyopencl()  # For OpenCL GPUs

    ## Transfer lattice on context and compile tracking code
    FULL_RING.build_tracker(_context=context)

    ## Compute lattice functions
    tw = FULL_RING.twiss(method='4d')
    tw_df = tw.to_pandas()
    tw_df.head()
    if verbose:
        print('TWISS')
        display(tw_df.head())
        # tw.show()

    fr_df = FULL_RING.to_pandas()
    mbeta, mgamma = particle_beta_gamma('mu+', mmomentum)
    revolution_period = FULL_RING.get_length()/(mbeta*scipy.constants.c)*1.0e6
    print(f'''LEnuSTORM Ring:
        elements defined : {fr_df.shape[0]}
        full length: {FULL_RING.get_length()}
        revoluiton period: {revolution_period} [us]
        ''')
    
    survey_df = FULL_RING.survey().to_pandas()
    
    lenustorm_df = fr_df.merge(survey_df, left_index=True, right_index=True, suffixes=('_fr','_srv'))
    lenustorm_df.head(20)
    
    lenustorm_df['X_cm'] = lenustorm_df['X'].apply(lambda x: x*100)
    lenustorm_df['Y_cm'] = lenustorm_df['Y'].apply(lambda x: x*100)
    lenustorm_df['Z_cm'] = lenustorm_df['Z'].apply(lambda x: x*100)
    lenustorm_df['theta_deg'] = lenustorm_df['theta'].apply(lambda x: x*180/np.pi)

    lenustorm_df['el_name'] = lenustorm_df['name_srv'].apply(lambda x: re.sub(r':+', '.', x).replace('_to_','2').replace('.fodoa','ac'))
    lenustorm_df['model_id'] = lenustorm_df.apply(lambda r: r.name_fr.split('.')[0] if r.element_type != 'Drift' else np.nan, axis=1)
    lenustorm_df['model_name'] = lenustorm_df['model_id'].apply(lambda x: x.replace('_to_','').lower()+'vol' if isinstance(x, str) else x) 
    lenustorm_df['ishalf'] = lenustorm_df.apply(lambda r: True if r.element_type != 'Drift' and r.drift_length == 0.125 else False, axis=1)
    lenustorm_df['Consecutive_Count'] = lenustorm_df.groupby((lenustorm_df['model_name'] != lenustorm_df['model_name'].shift()).cumsum()).cumcount() + 1

    return FULL_RING, tw_df, survey_df, lenustorm_df

def gen_t2rline(fin, verbose=False):
        
    TRANSLINE = xt.Line.from_json(fin)
    t2r_df = TRANSLINE.to_pandas()
    t2r_df['k0'] = t2r_df.apply(lambda r: r.element.k0 if r.element_type == 'Bend' else None, axis=1)
    t2r_df['k1'] = t2r_df.apply(lambda r: r.element.k1 if r.element_type == 'Quadrupole' else None, axis=1)
    t2r_df['name'] = t2r_df['name'].apply(lambda x: x.replace('T','').replace('.hs1','b'))
    t2r_df
    
    t2r_srv_df = TRANSLINE.survey().to_pandas()
    t2r_srv_df['name'] = t2r_srv_df['name'].apply(lambda x: x.replace('T','').replace('.hs1','b'))
    
    tlcomplete_df = t2r_df.merge(t2r_srv_df, left_index=True, right_index=True, suffixes=('_tl','_srv'))
    tlcomplete_df[tlcomplete_df.element_type != 'Drift']
    
    tlcomplete_df['X_cm'] = tlcomplete_df['X'].apply(lambda x: x*100)
    tlcomplete_df['Y_cm'] = tlcomplete_df['Y'].apply(lambda x: x*100)
    tlcomplete_df['Z_cm'] = tlcomplete_df['Z'].apply(lambda x: x*100)
    tlcomplete_df['theta_deg'] = tlcomplete_df['theta'].apply(lambda x: x*180/np.pi)

    tlcomplete_df['el_name'] = tlcomplete_df.apply(lambda r: r.name_srv.replace('_','') if not r.element_type in ['Drift', 'Marker'] else r.name_srv, axis=1)
    tlcomplete_df['model_id'] = tlcomplete_df.apply(lambda r: 'TTMB' if r.element_type == 'Bend' else 'TTMQ' if r.element_type == 'Quadrupole' else np.nan, axis=1)
    tlcomplete_df['model_name'] = tlcomplete_df['model_id'].apply(lambda x: x.replace('_','').lower()+'vol' if isinstance(x, str) else x) 
    tlcomplete_df['ishalf'] = tlcomplete_df.apply(lambda r: 0 if r.element_type != 'Quadrupole' else 1 if r.drift_length == 0.4 else 0, axis=1)


    _dmy = pd.DataFrame(tlcomplete_df['ishalf'])
    _dmy['counter'] = _dmy['ishalf'].apply(lambda x: None if x == 0 else x)
    _dmy['counter'] = _dmy.groupby('counter').cumcount() +1 
    _dmy['counter'] = _dmy['counter'].where(_dmy['ishalf'] != 0, 0).astype(int)  # Reset counter to 0 for rows with 0s

    tlcomplete_df['Consecutive_Count'] = _dmy['counter'].values
    return TRANSLINE, t2r_df, t2r_srv_df, tlcomplete_df
