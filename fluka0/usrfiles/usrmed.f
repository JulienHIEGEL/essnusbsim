*$ CREATE USRMED.FOR
*COPY USRMED
*                                                                      *
*=== usrmed ===========================================================*
*                                                                      *
      SUBROUTINE USRMED ( IJ, EKSCO, PLA, WEE, MREG, NEWREG, XX, YY, ZZ,
     &                    TXX, TYY, TZZ )

      INCLUDE 'dblprc.inc'
      INCLUDE 'dimpar.inc'
      INCLUDE 'iounit.inc'
*
*----------------------------------------------------------------------*
*                                                                      *
*     Copyright (C) 1991-2005      by    Alfredo Ferrari & Paola Sala  *
*     All Rights Reserved.                                             *
*                                                                      *
*                                                                      *
*     USeR MEDium dependent directives:                                *
*                                                                      *
*     Created on  10  may  1996    by    Alfredo Ferrari & Paola Sala  *
*                                                   Infn - Milan       *
*                                                                      *
*     Last change on   29-may-96   by    Alfredo Ferrari               *
*                                                                      *
*     Input variables:                                                 *
*             ij = particle id                                         *
*          Eksco = particle kinetic energy (GeV)                       *
*            Pla = particle momentum (GeV/c)                           *
*            Wee = particle weight                                     *
*           Mreg = (original) region number                            *
*         Newreg = (final)    region number                            *
*       Xx,Yy,Zz = particle position                                   *
*    Txx,Tyy,Tzz = particle direction                                  *
*                                                                      *
*     The user is supposed to change only WEE if MREG = NEWREG and     *
*     WEE, NEWREG, TXX, TYY, TZZ if MREG .NE. NEWREG                   *
*                                                                      *
*----------------------------------------------------------------------*
*

ccc Pions and Kaons exiting the target
      character*8 mregn, newregn

      call geor2n(mreg,mregn,ierr1)
      call geor2n(newreg,newregn,ierr1)

*     Score All particle coming out of the target frontier
*     --
      if((mregn.eq."TARGET").and.(newregn.ne."TARGET")) then

       write(40,1040) ij,Xx,Yy,Zz,Pla,Tzz,Tyy,Tzz

       if (ij.eq.13) then
         theta=acos(Tzz)
         write(41,1041,err=900) pla,theta
       endif

       if (ij.eq.14) then
        theta=acos(Tzz)
        write(42,1042) Pla,theta
       endif

      endif

*     Score All particle at the beginning of the tunnel
*     --
      if((mregn.eq."EXPAREA").and.(newregn.eq."DT")) then

        theta_x = acos(Txx)
        theta_y = acos(Tyy)
        theta_z = acos(Tzz)

        pla_x = pla*Txx
        pla_y = pla*Tyy
        pla_z = pla*Tzz

*       if ( (ij.eq.13).and.(tzz.gt.0) ) then
       if (ij.eq.13) then
         write(50,1050) ij,Xx,Yy,Zz,Pla,Tzz,Tyy,Tzz
         theta=acos(Tzz)
         pid = 211
*         write(51,1051,err=900) pla,theta
         write(51,1051,err=900) Xx,Yy,Zz,pla_x,pla_y,pla_z,0.0,pid,0.0,0.0,0.0,0.0
       endif

       if(ij.eq.14) then
        theta=acos(Tzz)
        pid = -211
*        write(52,1052) Pla,theta
         write(52,1052,err=900) Xx,Yy,Zz,pla_x,pla_y,pla_z,0.0,pid,0.0,0.0,0.0,0.0
       endif

      endif

 900    continue
 1040   format(I4,7(1x,e14.7))
 1041   format(2(1x,e14.7))
 1042   format(2(1x,e14.7))
 1050   format(I4,7(1x,e14.7))
 1051   format(12(1x,e14.7))
 1052   format(12(1x,e14.7))

      RETURN
*=== End of subroutine Usrmed =========================================*
      END
