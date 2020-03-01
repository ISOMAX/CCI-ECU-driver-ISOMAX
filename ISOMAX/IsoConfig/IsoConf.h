/* ************************************************************************** */
/*!
   \file        IsoConf.h  
               
   \brief       Sample Configuration file for ISO11783 Driver 
                - should be modified by the application programmer 
   
   File for individual configuration settings of ISO Driver - Version for WHEPS Demo
   
   
   \author      Hammerl Erwin
   \date        Created 29.06.2006 \n
                Updated 04.11.2016

   \copyright   Competence Center ISOBUS e.V.
   
*/
/* ************************************************************************** */
#ifndef DEF_ISO_CONF_H
   #define DEF_ISO_CONF_H

/* ************************************************************************** */
/*! \addtogroup isoconf IsoConf.h Sample
    \ingroup  isoconfig 
    \brief    Sample configuration file
    \details  To define an other file than IsoConf.h as configuration file 
              add the define ISOCONF_APPFILE to your application.
    \code
    #define ISOCONF_APPFILE "MyIsoConf.h"
    \endcode
*/
/*! @{ */
/* ************************************************************************** */
//#define ISO_DEBUG_ENABLED
#define ISO_SECONDARY_POOL_MANIPULATION
/* ************************************************************************** */
/*!
  \def HUGE_C
  \brief Memory Type for pointers to huge size variables.
*/
#ifdef __C166__                                 /*Keil*/
       #define   HUGE_C        xhuge
#else  
    #ifdef _C166                                /*Tasking 16 bit*/    
       #define   HUGE_C        huge
    #else
/*       #define   HUGE_C define only if needed */       
    #endif
#endif

#define ISO_VTC_ENABLE_ALARM
#define ISO_EXTERNAL_POOLBUFFER
#define ISO_EXTERNAL_DDOPBUFFER
/* ************************************************************************** */
/*! \name Defines - Enable Clients, protocols, layers, ...
    To disable a functionality and compilation remove the corresponding define.
*/
/*! @{ */

#define _LAY78_   /**< \brief  Enable Part 7: \link apla  Multi CAN Node Application Layer \endlink functionality 
                       \details Since version 8.0 the Base Driver Multi CAN Node API is used as default. \n
                                Removing _LAY78_ from the configuration disables only the application layer module compilation. \n */
#define _LAY6_    /**< \brief Enable Part 6:  \link vtcl   Virtual Terminal Client \endlink functionality */
#define _LAY10_   /**< \brief Enable Part 10: \link taskc  Task Controller Client \endlink functionality */
//#define _LAY13_   /**< \brief Enable Part 13: \link fsclient  File Server Client \endlink functionality */
//#define _LAY14_   /**< \brief Enable Part 14: \link sequencectrl  Sequence Control Client \endlink functionality (needs _LAY6_) */
#define _NMEA_    /**< \brief Enable NMEA 2000 extension: fast packet transfer, Module IsoDataM.c is necessary */

#ifdef DOXYGEN  /* add define to documentation */     
    //#define _LAY4_   /**< \brief Enable Part 4: \link niuapi Network Interconnection Unit \endlink service functionality 
                          \details Use the defines in the \ref niuconfig "configuration" section to configure the service. */ 
#endif
/*!@} */

/* ************************************************************************** */
/** \name Defines - Base Driver Multi CAN Node configuration (optional)
  @{ */
/*! [CAN Node configuration] */
/*! \def ISO_CAN_NODES 
   \brief Number of physical CAN-busses which shall work with the J1939/ISO11783 protocol ( 1, 2, ...) (default 1) */
#define ISO_CAN_NODES   1
       
/*! \def ISO_CAN_VT 
   \brief   CAN Node which supports VT, TC, SC, FS functionality (default 0).
   \details Number of the CAN-bus with full ISO-functionality. \n
				This CAN bus node is used for all clients ( VT-Client, TC-Client, ... ). \n
            All other busses support only basic-functionality. 
            Range: 0, 1, ... , (ISO_CAN_NODES-1) 
*/
#define ISO_CAN_VT      0u  

/*! [CAN Node configuration] */
/* not used in the driver, only a sample */
#define   ISO_CAN_NODE1     0u          /**< \brief CAN 1 */ 
#define   ISO_CAN_NODE2     1u          /**< \brief CAN 2 .... */ 
#define   ISO_CAN_NODE3     2u          /**< \brief CAN 3 .... */ 
#define   ISO_CAN_NODE4     3u          /**< \brief CAN 4 .... */ 
/*! @} */

/* ************************************************************************** */
/** \name Defines - Base Driver Part 5: Network management configuration (optional)
  @{  */
/*! \def ISO_PART5_FE
   \brief   Enable the obsolete behavior of the first edition of Part 5 (not recommended).
   \details For further information see \link drivernodeconfig Part 5 Configuration \endlink
*/
#ifdef DOXYGEN
#define ISO_PART5_FE
#endif

/* The self-configurable address range is between 128 and 247 */
/* Constriction because of conformance test */
#define ISO_FIRST_FREE_SA  200u     /**< \brief First self-configurable address - predefined with 200u */
#define ISO_LAST_FREE_SA   220u     /**< \brief Last  self-configurable address - predefined with 220u */

#define ISO_USER_MAX         64      /**< \brief   Default value for the maximum number of network members for a CAN Node - predefined with 32
                                          \details This value is used as maximum number of entries in the address table for a node,
                                                   if nothing else is specified. To set the maximum number of network members entries
                                                   for a specific CAN node, you can add #ISO_USER_MAX_NODE1, #ISO_USER_MAX_NODE2, ..., #ISO_USER_MAX_NODE6
                                                   to this configuration file. */
													 
#define ISO_NM_LOOPTIME    (15L)   /**< \brief Loop time of the ISO-Task in order to compensate negative effects on 
                                               wait-times - predefined with 10 */
													 
#ifdef DOXYGEN   /* add defines to documentation */
    #define ISO_USER_MAX_NODE1      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 1 */
    #define ISO_USER_MAX_NODE2      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 2 */
    #define ISO_USER_MAX_NODE3      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 3 */
    #define ISO_USER_MAX_NODE4      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 4 */
    #define ISO_USER_MAX_NODE5      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 5 */
    #define ISO_USER_MAX_NODE6      ISO_USER_MAX    /**< \brief Maximum number of network members for CAN Node 6 */
#endif

/*! @} */

/* ************************************************************************** */
/** \name Defines - Base Driver Part 3: Transport protocol configuration (optional) 
  @{  */

/*! \def TP_PARALLEL_MAX
   \brief   Parallel transport channels for all CAN-nodes - predefined with 5 
   \details Number of transport protocols which run simultaneously respectively which are saved 
            temporarily (same sender and receiver). This parameter defines the transport channels
            for all CAN-nodes ! ( in contrary to version before 6.01 - for every CAN-node ) */
#define TP_PARALLEL_MAX     10  

/* Optional for version >= 6.02 -> Saving RAM, enhancing number of channels per CAN-node  */

/*! \def ISO_TPREDUCED_MAX
   \brief   Optional: Number of transport channels with reduced buffer size - predefined with 0
   \details This parameter defines the number of transport channels with reduced buffer size.
            #TP_PARALLEL_MAX - #ISO_TPREDUCED_MAX are the channels with complete TP size (1785bytes)
            For setting reduced size see BLOCK_SIZE_J1939 */
#define ISO_TPREDUCED_MAX    5   

/*! \def BLOCK_SIZE_J1939
   \brief   Optional: Reduced data buffer size for TPs - predefined with 1785 Bytes  									  
   \details Defines the data buffer size for the with #ISO_TPREDUCED_MAX defined TP channels. 
            Range: 9 to 1785 bytes \n
            E. g. 223 corresponds to the max. NMEA FPT size */
#define BLOCK_SIZE_J1939   223u   

#ifdef DOXYGEN     /* Used for creating the documentation ! */
   #define _J1939_   /**< \brief   obsolete - normally not needed 
                          \details Default priority of connection management (TP) is switched to J1939 style. */
#endif

/*! @} */

/* ************************************************************************** */
/** \name Defines - Base Driver Part 7: Application layer configuration (optional) 
  @{  */
#define NUMBERPGN_MAX      40    /**< \brief Maximum number of PGN handles - predefined with 10 
                                    \details Number of PGNs which can be initialised in the revised application layer */

#define NUMBERSPN_MAX      60    /**< \brief Maximum number of SPNs - predefined with 20
											  \details Number of SPNs which can be initialised in the revised application layer 
                                   - SPN-Support deactivation: Set NUMBERSPN_MAX to 0  */
#ifdef DOXYGEN   /* add defines to documentation */
    #define NO_SPN_DEFINES       /**< \brief Do not use the SPN defines of the ECU Driver */
#endif
/*! @} */ 

/* END base driver */

/* ************************************************************************** */
/** \name Defines - Part 6 Virtual Terminal Client (LAYER 6) 
    See  \link vtclmem Reduce memory footprint options \endlink for 
    options to disable or enable specific sub modules or functionalities.  
*/
/*! @{ */

/** \def WORKING_SETS 
    \brief Number of possible working sets 
 
*/
#define   WORKING_SETS          3		/**< \brief Working Set with auxiliary functions type 2 needs 2 Number of Working Sets -
											  each working set with auxiliary function increases this value by 2, without by 1  */

#define   WS_EXTERN            10      /**< \brief All extern Working sets ( auxiliary units, workings sets ) - predefined with 10 */


/* Define IsoVt.c */
#define   VTMSG_BUFSIZE        (70u)      /**< \brief Command buffer - normal size 

										Size of the command message buffer. When opening a data mask, usually a command 
										is sent for each object. The buffer must be bigger than this number. */

#define   STRBUFFSIZE          10      /**< \brief Number of strings which can be stored 
										
										Number of strings which can be saved temporarily for a command buffer entry 
										(> maximum number of strings which are on one page) */

#define   STRLENGTHMAX         (70u)      /**< \brief String length - Maximal size of a single string 
										
										This buffer must be bigger than the longest string present (in bytes), 
										which is sent via the bus. */

#define   ISO_WAITVT_SEC       (5u)    /**< \brief Waittime in seconds for preferred VT - predefined with 7
										
										Length of time in seconds, equals waiting period for the preferred terminal 
										(see IsoVtInit() ), pre-defined with 7 seconds */

#define   POOLVERSIONS          (10u)     /**< \brief Max. number of version strings stored after get version - predef. with 5 
										
										This Parameter defines the number of pool versions which are stored in the VTClient
										when receiving the get version command */  

#define   POOBBUFF              (60uL)     /**< \brief Size of buffer for WSobject - predefined with 30 */

/* Define IsoPool.c */
#define   RAMSIZEPOOL                 5108     /**< \brief Size of buffer for segmented pool transfer

													Size of memory in bytes for objects which are being sent with a single ETP. 
													In case there is sufficient RAM, the buffer should correlate to the pool size 
													as some terminals display a charge indicator which applies to the whole pool 
													and the whole pool is being downloaded in an ETP. In any case this buffer 
													must be bigger than the biggest object in the pool (256 colour-images!). */

#define   SEGPOOLBLOCK                 1000u    /**< \brief Block request during segmented pool transfer
                                       
                                       Number of bytes requested of driver of "flash" - depends on the OS */

#define   ISO_NO_SCALING_ELEMENTS_MAX  (30u)     /**< \brief Maximal number of manipulation ranges
													
													Maximum number of ranges given for pool manipulation 
													( Number of calling IsoPoolSetIDRangeMode() ) */
													
#define   ISO_MANIPULATE_MAX_LOOPS     10    /**< \brief Maximum number of objects loaded, manipulated .. to RAM during one loop
                                       
                                       Number of manipulated objects during one machine loop - default is 4 */

#define   ISO_MANIPULATE_MAX_TIME_MS   100    /**< \brief Maximum time in ms which driver can load objects before next loop 
                                       
                                       Maximum time in ms for manipulating objects till driver leave this loop - default is 300 ms */

#define   ISO_POOLNUMANICOPY             4    /**< \brief Maximum number of multiple copied objects including original */

/* Define IsoVtXcr.c */
/** \def MASKBITSNUMBER
    \brief      For size of array for automatic responses - predefined with 6 
 	
*/
#define   MASKBITSNUMBER                 0x06u    /**< \brief Number of maskbits -> array size ( 2 ^ MASKBITSNUMBER ) + 20 

										 The maximum number of objects which can be sent to a data/alarm mask by using special 
										 command functions ( see IsoCommandEC(), IsoCommandTC()  ... ), is calculated with this parameter.
                               These functions use a key search algorithm, the number of mask bits determine the array size. 
										 The latter is calculated with the formula (2 ^ MASKBITSNUMBER + 20), 
										 i.e. MASKBITSNUMBER = 6 equals 84 elements, MASKBITSNUMBER = 7 equals 148 elements. */

/* Define IsoAux.c */    
#define   AUXINPUTMAX                 (20u)     /**< \brief Number of auxiliary functions (in the working set pool, predefined with 40 

													Maximum number of auxiliary functions (with auxiliary devices auxiliary inputs) 
													in the pool */

#define   BUFF_PREASS                   300     /**< \brief Buffer for sending preferred assignment to the VT - predefined with 200 

													Internal buffer for preferred assignment (approx. 15 bytes per allocation 
													is sufficient)*/

/*! @} */ /* END Part 6 */

/* ************************************************************************** */
/** \name Defines - Part 10 Task Controller Client */
/** @{ */
#define   TC_NUMB_TCCLIENTS               3      /**< \brief Defines the number of possible TC-Clients */

#define   PROCESSDATA_MAX                100      /**< \brief Maximum number of process data ( predefined is 20)

														(must be higher than the real number PDs). */

#define   DEVICEDESCRIPTION_SIZE        1610     /**< \brief Size of buffer for device description pool ( predefined is 1000)

												RAM-buffer for the DeviceDescription (number DD-objects * 50 should be sufficient)
                                    - Minimum is real size + 50 */

#define   TC_NUMNESTED_DE                10      /**< \brief Number of nested Device elements ( predefined with 10 ) */

#define   TC_TIMECYCLEMIN_PDS           (50L)    /**< \brief Minimum timecycle for PD list ( reducing prozessor load )  */

/** @} */ /* END Part 10 */

/* ************************************************************************** */
/** \name Defines - Part 13 File Server Client (optional)
     see \link fscconfig FS-Client Configuration \endlink for a list of all defines \n
     Configuration Sample: 
*/
/*! @{ */
#define FSC_MAX_CONNECTIONS     2       /**< \brief Maximum of simultaneous connections ( predefined is 2)  */               
#define FSC_MAX_IO_HANDLES      6       /**< \brief Maximum of simultaneous io channels ( predefined is FSC_MAX_CONNECTIONS * 3)  */     
/*! @} */ /* END Part 13 */

/* ************************************************************************** */
/** \name Defines - Part 14 Sequence Control Client */
/*! @{ */
#define SCD_SIZE              1000     /**< \brief  Predefined size of SCD buffer is 1000 */
#define SCFUNCTION_MAX        10       /**< \brief  Predefined number of functions is 20 
                                            \details Maximum number of SCD function objects (more than in SCD) */
#define SCSTATES_MAX          15      /**< \brief   Predefined number of functions is 20
                                           \details  Maximum number of SCD state objects (more than in SCD) */
#define  SCPOOBBUFFEXT        20      /**< \brief  Size of buffer for ExtDefObjDef object - predefined with 30 */
/*!@} */ /* END Part 14 */


#ifdef ISO_BASE_DISTRIBUTOR_OLD  /* deprecated defines used from old base distributor */  
/* ************************************************************************** */
/*! \name Defines - Enable message forwarding to the data link callback function for specific layers 
      The following defines can be added to the IsoConf.h to receive PGN messages which are normally 
      forwarded to a client with the CF data link callback function.
      It is recommended that the corresponding client (layer) is deactivated in this case.
*/
/*! @{ */
#define ISO_DLCB_LAY6	/*!< \brief Forward #PGN_ECUtoVT and #PGN_VTtoECU messages to CF data link callback function */
#define ISO_DLCB_LAY10	/*!< \brief Forward #PGN_PROCESS_DATA messages to CF data link callback function */
#define ISO_DLCB_LAY13	/*!< \brief Forward #PGN_FS_TO_FSC and #PGN_FSC_TO_FS messages to CF data link callback function */
#define ISO_DLCB_LAY14	/*!< \brief Forward #PGN_SCM_TO_SCC and #PGN_SCC_TO_SCM messages to CF data link callback function */
/*!@} */
/*! \name Defines - Enable message forwarding to the application layer
The following defines can be added to the IsoConf.h to receive PGN messages which are normally
forwarded to a client with the application layer module. 
The corresponding client (layer) can not be used in this case, because the client message processing is skipped.
*/
/*! @{ */
#define ISO_OMIT_VTC_PGNS /*!< forward PGN_VTtoECU and PGN_ECUtoVT messages to "application layer" */
#define ISO_OMIT_TCC_PGNS /*!< forward PGN_PROCESS_DATA messages to "application layer" */
#define ISO_OMIT_SCC_PGNS /*!< forward PGN_SCM_TO_SCC and PGN_SCC_TO_SCM messages to "application layer" */
#define ISO_OMIT_FSC_PGNS /*!< forward PGN_FSC_TO_FS and PGN_FS_TO_FSC messages to "application layer" */
#define ISO_MODULE_APPLAYER_OMIT_RETURN_VALUE /*!< CCI workaround: Handle TP message in application layer and network layer callback*/
/*!@} */
#undef ISO_OMIT_VTC_PGNS
#undef ISO_OMIT_TCC_PGNS
#undef ISO_OMIT_SCC_PGNS
#undef ISO_OMIT_FSC_PGNS
#undef ISO_MODULE_APPLAYER_OMIT_RETURN_VALUE


#endif

/* ************************************************************************** */
/*!@}*/ /* END doxygen group isoconf */
/* ************************************************************************** */

#endif   /* DEF_ISO_CONF_H */
