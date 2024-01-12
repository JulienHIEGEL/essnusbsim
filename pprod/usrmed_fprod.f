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
      
      LOGICAL LFIRST
      DATA  LFIRST /.TRUE./

      SAVE LFIRST,NT2HEAD, NT2INSHLD, NT2COLL, NBEFB2, NNA61
      SAVE NAIRAFTER, NB3TVAC, NBEFTAX

      IF (LFIRST) THEN
         LFIRST = .FALSE.
         CALL GEON2R("T2HEAD  ", NT2HEAD,IERR)
         CALL GEON2R("T2INSHLD", NT2INSHLD, IERR)
         CALL GEON2R("T2COLL  ", NT2COLL, IERR)
         CALL GEON2R("B3TVac  ", NB3TVAC, IERR)
         CALL GEON2R("AIRBEFB2", NBEFB2, IERR)
         CALL GEON2R("AIRAFTER", NAIRAFTER, IERR)
         CALL GEON2R("NA61exp ", NNA61, IERR)
         CALL GEON2R("VXSS    ", NVXSS, IERR)
         CALL GEON2R("BEFTAX  ", NBEFTAX,IERR)

         PRINT *,"Entering USRMED routine",MREG,NEWREG
         PRINT *,"Named regions : ",NT2HEAD, NT2INSHLD, NT2COLL, NBEFB2, NNA61
         PRINT *,"                ",NBEFB2, NVXSS, NAIRAFTER, NBEFTAX

         WRITE(95,*) "* 1=IJ 2=PLA 3-5={Xx,Yy,Zz},6-8={TXX,TYY,TZZ}, 9=WEE"
      END IF

*C --- keep only pions, muons & kaons
*
*      IF ( IJ.EQ.1 .OR. IJ.LT.-6 ) THEN
*         WEE = ZEROZERO
*         RETURN
*      END IF

*C --- calculate particle momentum
*
*      IF ( PLA.LT.0.0D0 ) THEN
*         EKIN = ABS(PLA)
*      ELSE IF ( PLA.GT.0.0D0 ) THEN
*         IF ( (PLA**2+AM(IJ)**2).GT.0.0 ) THEN
*            EKIN = SQRT((PLA**2+AM(IJ)**2))-AM(IJ)
*         ELSE
*            EKIN = 0.0
*         ENDIF
*      ELSE
*         RETURN
*      END IF
*      IF ( (EKIN+AM(IJ)**2-AM(IJ)**2).GT.0.0 ) THEN
*         PMOMZ = SQRT((EKIN+AM(IJ))**2-AM(IJ)**2)/FLOAT(ICHRGE(IJ))
*         PMOMA = SQRT((EKIN+AM(IJ))**2-AM(IJ)**2)/FLOAT(IBARCH(IJ))
*      ELSE
*         PMOMZ = 0.0
*         PMOZA = 0.0
*      ENDIF

      IF ( MREG.EQ.NT2HEAD .AND. NEWREG.EQ.NT2INSHLD ) THEN

*     --- score particles T2 shielding area, surrounding the target
         
         IF (IJ.EQ.1 .OR. IJ.EQ.2 .OR. (IJ.GT.10 .AND. IJ.LE.16) ) THEN 
            WRITE(95,1000) IJ, PLA, Xx,Yy,Zz,TXX,TYY,TZZ,WEE
         ENDIF
*     ELSE IF (MREG.EQ.NB3TVAC .AND. NEWREG.EQ.NBEFB2) THEN
*  --- particles before degrader
*
*         WRITE(96,1000) IJ,ICHRGE(IJ),IBARCH(IJ),AM(IJ),EKSCO,PLA,
*     +        PMOMZ,PMOMA,Xx,Yy,Zz,TXX,TYY,TZZ,WEE
*      ELSE IF (MREG.EQ.NAIRAFTER .AND. NEWREG.EQ.NNA61) THEN

C --- particles after degrader

*         WRITE(97,1000) IJ,ICHRGE(IJ),IBARCH(IJ),AM(IJ),EKSCO,PLA,
*     +        PMOMZ,PMOMA,Xx,Yy,Zz,TXX,TYY,TZZ,WEE
*      ELSE IF (MREG.EQ.NVXSS .AND. NEWREG.EQ.NBEFTAX) THEN
C --- particles beftax
* 
*         WRITE(98,1000) IJ,ICHRGE(IJ),IBARCH(IJ),AM(IJ),EKSCO,PLA,
*     +        PMOMZ,PMOMA,Xx,Yy,Zz,TXX,TYY,TZZ,WE
       
      END IF
      RETURN
 1000 FORMAT(1(1X,I4),8(1X,G14.7))
*=== End of subroutine Usrmed =========================================*
      END

