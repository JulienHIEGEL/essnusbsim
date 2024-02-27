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

      
      SAVE LFIRST,NTARGET,NVOID
      
      IF (LFIRST) THEN
         LFIRST = .FALSE.
         CALL GEON2R("TARGET  ", NTARGET,IERR)
         CALL GEON2R("VOID    ", NVOID, IERR)
        
         PRINT *,"Entering USRMED routine",MREG,NEWREG
         PRINT *,"Named regions : ",NVOID,NTARGET
         PRINT *,"Configuration : IFOCUSING=",IFOCUSING
         WRITE(95,*) "* 1=IJ 2=PLA 3-5={Xx,Yy,Zz},6-8={TXX,TYY,TZZ}, 9=WEE"
      END IF

      IF ( (MREG.EQ.NTARGET) .AND. (NEWREG.EQ.NVOID) ) THEN
            IF ( (IJ.EQ.13).OR.(IJ.EQ.14).OR.(IJ.EQ.15).OR.(IJ.EQ.16) ) THEN    
                  WRITE(95,1000) IJ, PLA, Xx,Yy,Zz,TXX,TYY,TZZ,WEE
                  IF ( IFOCUSING.EQ.1 ) THEN
                        TXX = ZERZER
                        TYY = ZERZER
                        TZZ = ONEONE
                  END IF
            END IF       
      END IF
      RETURN
 1000 FORMAT(1(1X,I4),8(1X,G14.7))
*=== End of subroutine Usrmed =========================================*
      END

