*
*=== usrini ===========================================================*
*
      SUBROUTINE USRINI ( WHAT, SDUM )

      INCLUDE 'dblprc.inc'
      INCLUDE 'dimpar.inc'
      INCLUDE 'iounit.inc'
*
*----------------------------------------------------------------------*
*                                                                      *
*     Copyright (C) 2003-2019:  CERN & INFN                            *
*     All Rights Reserved.                                             *
*                                                                      *
*     USeR INItialization: this routine is called every time the       *
*                          USRICALL card is found in the input stream  *
*                                                                      *
*     Created on 01 January 1991   by    Alfredo Ferrari & Paola Sala  *
*                                                   Infn - Milan       *
*                                                                      *
*----------------------------------------------------------------------*
*
      DIMENSION WHAT (6)
      CHARACTER SDUM*8

      COMMON / MYSIMULATION / IFOCUSING
      SAVE / MYSIMULATION /
      
*
*  Don't change the following line:
      LUSRIN = .TRUE.
*     *** Write from here on *** *
      IFOCUSING = WHAT(1)
      PRINT *,'--- USRINI: input parameters ',IFOCUSING
      RETURN
*=== End of subroutine Usrini =========================================*
      END

