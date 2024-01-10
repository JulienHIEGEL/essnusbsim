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
*
*  Don't change the following line:
      LUSRIN = .TRUE.
* *** Write from here on *** *

*----------------------------------------------------------------------*
*     
*    Data file definition
*    --
*    40 : Contain all particle existing the target
*         Filled in usermed.f
*    41 : Contain all pi+ distribution exiting the target 
*         Filled in usermed.f
*----------------------------------------------------------------------*

      open (unit = 40, file = 'allpid', status = 'UNKNOWN')
      open (unit = 41, file = 'piplus_outoftarget', status = 'UNKNOWN')
      open (unit = 42, file = 'piminus_outoftarget', status = 'UNKNOWN')

      open (unit = 50, file = 'allpid_in_dt', status = 'UNKNOWN')
      open (unit = 51, file = 'piplus_in_dt', status = 'UNKNOWN')
      open (unit = 52, file = 'piminus_in_dt', status = 'UNKNOWN')

      RETURN
*=== End of subroutine Usrini =========================================*
      END

