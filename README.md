# T2R and LEnuSTORM Simulation

ESSnuSB+ Simulation Studies for the LEnuSTORM stage

## Structure

- **fluka0** initial FLUKA simulation code for target and decay pipe
- **pprod** particle production out of the target
- **raytrace_cpp** tracking code developed for LAGUNA-LBNO CN2PY design studies
- **lenustorm** neutrino beam to LEnuSTORM


## Script to generate the FLUKA Simulation Input

- **dev/generate_fluka.ipynb** : basic notebook file
    Input : beam files (optics) from the src direcory.
    Output: FLUKA output files *.inp in the same directory
    Status: see comments directly in the file

- **cp_inp_files.sh** : script to copy the new files from the dev directory to the main one where the basic FLUKA/FLAIR project files are. 