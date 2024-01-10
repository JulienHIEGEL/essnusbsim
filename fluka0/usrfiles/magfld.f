*
*===magfld=============================================================*
*
      SUBROUTINE MAGFLD ( X, Y, Z, BTX, BTY, BTZ, B, NREG, IDISC )

      INCLUDE 'dblprc.inc'
      INCLUDE 'dimpar.inc'
      INCLUDE 'iounit.inc'
*
*----------------------------------------------------------------------*
*                                                                      *
*     Copyright (C) 2003-2019:  CERN & INFN                            *
*     All Rights Reserved.                                             *
*                                                                      *
*     Created  in     1988         by    Alberto Fasso`                *
*                                                                      *
*     Input variables:                                                 *
*            x,y,z = current position                                  *
*            nreg  = current region                                    *
*     Output variables:                                                *
*            btx,bty,btz = cosines of the magn. field vector           *
*            B = magnetic field intensity (Tesla)                      *
*            idisc = set to 1 if the particle has to be discarded      *
*                                                                      *
*----------------------------------------------------------------------*
*
      INCLUDE 'cmemfl.inc'
      INCLUDE 'csmcry.inc'

      double precision bx, by, bz, current, r2
*
*  +-------------------------------------------------------------------*
*  |  Earth geomagnetic field:
      IF ( LGMFLD ) THEN
         CALL GEOFLD ( X, Y, Z, BTX, BTY, BTZ, B, NREG, IDISC )
         RETURN
      END IF
*  |
*  +-------------------------------------------------------------------*

      sigcurrent = 1.
      current = 350.

      IDISC = 0

* Horn's magnetic field  I<0 positive focusing
* --
      r2 = x*x+y*y
      bx = -2.*1D-02*sigcurrent*current*y/r2
      by = 2*1D-02*sigcurrent*current*x/r2
      bz = 0.
      b = sqrt(bx*bx+by*by)
      btx = bx/b
      bty = by/b
      btz = bz/b
      RETURN
*=== End of subroutine Magfld =========================================*
      END

