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
      INCLUDE 'paprop.inc'

      COMMON / MYSIMULATION / IFOCUSING
      
      LOGICAL LFIRST
      DATA  LFIRST /.TRUE./
      
      SAVE LFIRST,NTARGET,NVOID,NTBOX,NHORN,NDET0,NDET10,NDET20,NDET30
      
      IF (LFIRST) THEN
         LFIRST = .FALSE.
         CALL GEON2R("TARGET  ", NTARGET,IERR)
         CALL GEON2R("VOID    ", NVOID, IERR)
         CALL GEON2R("TBOX    ", NTBOX, IERR)
         CALL GEON2R("HORN    ", NHORN, IERR)
         CALL GEON2R("DET0    ", NDET0, IERR)
         CALL GEON2R("DET10   ", NDET10, IERR)
         CALL GEON2R("DET20   ", NDET20, IERR)
         CALL GEON2R("DET30   ", NDET30, IERR)
        
         PRINT *,"Entering USRMED routine",MREG,NEWREG
         PRINT *,"Named regions : ",NVOID,NTARGET,NTBOX,NHORN,NDET0,NDET10,
     +                              NDET20,NDET30
         PRINT *,"Configuration : IFOCUSING=",IFOCUSING
         WRITE(95,*) "* 1=IJ 2=PLA 3-5={Xx,Yy,Zz},6-8={TXX,TYY,TZZ}, 9=WEE"
      END IF

      IWRITE = 0
      IF ( (MREG.EQ.NTARGET).AND.(NEWREG.EQ.TBOX) ) THEN
            IWRITE = 95
      ELSE IF ( (MREG.EQ.VOID).AND.(NEWREG.EQ.DET0) ) THEN
            IWRITE = 96
      ELSE IF ( (MREG.EQ.VOID).AND.(NEWREG.EQ.DET10) ) THEN
            IWRITE = 97
      ELSE IF ( (MREG.EQ.VOID).AND.(NEWREG.EQ.DET20) ) THEN
            IWRITE = 98
      ELSE IF ( (MREG.EQ.VOID).AND.(NEWREG.EQ.DET30) ) THEN
            IWRITE = 99
      ELSE
            IWRITE = 0
      END IF
      IF ( (IWRITE.GT.0).AND.
     +     ((IJ.EQ.1).OR.(IJ.EQ.2).OR.(IJ.EQ.10).OR.(IJ.EQ.11).OR.
     +       (IJ.EQ.13).OR.(IJ.EQ.14)) ) THEN    
            WRITE(IWRITE,1000) IJ, PLA, Xx,Yy,Zz,TXX,TYY,TZZ,WEE
      END IF

      IF ( (MREG.EQ.TBOX).AND.(NEWREG.EQ.HORN).AND.
     +     (IFOCUSING.EQ.1).AND.(Tzz.GT.ZERZER).AND.
     +     ((IJ.EQ.1).OR.(IJ.EQ.2).OR.(IJ.EQ.10).OR.(IJ.EQ.11).OR.
     +           (IJ.EQ.13).OR.(IJ.EQ.14)) ) THEN
            Txx = ZERZER
            Tyy = ZERZER
            Tzz = ONEONE
      END IF     

      RETURN
 1000 FORMAT(1(1X,I4),8(1X,G14.7))
*=== End of subroutine Usrmed =========================================*
      END

