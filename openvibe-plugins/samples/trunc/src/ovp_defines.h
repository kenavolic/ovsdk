#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ScenarioExporterXMLDesc                 OpenViBE::CIdentifier(0x5B2B5C67, 0x78EA7539)
#define OVP_ClassId_ScenarioImporterXMLDesc                 OpenViBE::CIdentifier(0x28DC180C, 0x4CCA7314)

#define OVP_ClassId_CrashingBoxDesc                         OpenViBE::CIdentifier(0x009F54B9, 0x2B6A4922)
#define OVP_ClassId_BoxAlgorithmProducerDesc                OpenViBE::CIdentifier(0x10C620AE, 0x211274FF)
#define OVP_ClassId_BoxAlgorithmConsumerDesc                OpenViBE::CIdentifier(0x191F4A6F, 0x07EB1BF2)
#define OVP_ClassId_SinusSignalGeneratorDesc                OpenViBE::CIdentifier(0x2633AFA2, 0x6974E32F)
#define OVP_ClassId_TimeSignalGeneratorDesc                 OpenViBE::CIdentifier(0x57AD8655, 0x1966B4DC)
#define OVP_ClassId_IdentityDesc                            OpenViBE::CIdentifier(0x54743810, 0x6A1A88CC)


/*
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00780136, 0x57633D46)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0024B879, 0x33C0E15F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00ABDABE, 0x41381683)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0088408E, 0x16E1484F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x004CF190, 0x502BAB67)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00A97AFA, 0x0746A776)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0075BBFB, 0x573AD9CC)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0026E131, 0x74171C97)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0027B51A, 0x6A69739E)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x007111B4, 0x0A320CB2)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00A184EE, 0x240933DA)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00EE3E2D, 0x08AD06AE)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x000F5246, 0x385FB6C9)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00D48FAC, 0x3A3B6BD3)
*/

//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_ScenarioExporterXML                     OpenViBE::CIdentifier(0x77075B3B, 0x3D632492)
#define OVP_ClassId_ScenarioImporterXML                     OpenViBE::CIdentifier(0x440BF3AC, 0x2D960300)

#define OVP_ClassId_CrashingBox                             OpenViBE::CIdentifier(0x00DAFD60, 0x39A58819)
#define OVP_ClassId_BoxAlgorithmProducer                    OpenViBE::CIdentifier(0x77A308C8, 0x711D8E57)
#define OVP_ClassId_BoxAlgorithmConsumer                    OpenViBE::CIdentifier(0x271A6EEC, 0x7B1D847E)
#define OVP_ClassId_SinusSignalGenerator                    OpenViBE::CIdentifier(0x7E33BDB8, 0x68194A4A)
#define OVP_ClassId_TimeSignalGenerator                     OpenViBE::CIdentifier(0x28A5E7FF, 0x530095DE)
#define OVP_ClassId_Identity                                OpenViBE::CIdentifier(0x5DFFE431, 0x35215C50)

/*
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00BE3E25, 0x274F2075)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0053270F, 0x78887C71)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00777FA0, 0x5DC3F560)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00CF6C44, 0x04D26C7D)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00164A4A, 0x1C7BDC72)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00372343, 0x71C98312)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0025722E, 0x0567E505)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00ACED37, 0x52569424)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00FF7C87, 0x769AFFCE)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00D522A6, 0x3AB5D79D)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00EDA10E, 0x5D78A41B)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00F4002F, 0x1F74BF6F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x003F7061, 0x0AF7719F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00C1AE03, 0x7ECAB994)
*/

//___________________________________________________________________//
//                                                                   //
// Operating System identification                                   //
//___________________________________________________________________//
//                                                                   //

// #define OVP_OS_Linux
// #define OVP_OS_Windows
// #define OVP_OS_MacOS
// #define OVP_OS_

#if defined TARGET_OS_Windows
 #define OVP_OS_Windows
#elif defined TARGET_OS_Linux
 #define OVP_OS_Linux
#elif defined TARGET_OS_MacOS
 #define OVP_OS_MacOS
#else
 #warning "No target operating system defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// Hardware Architecture identification                              //
//___________________________________________________________________//
//                                                                   //

// #define OVP_ARCHITECTURE_i386
// #define OVP_ARCHITECTURE_

#if defined TARGET_ARCHITECTURE_i386
 #define OVP_ARCHITECTURE_i386
#else
 #warning "No target architecture defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// Compilator software identification                                //
//___________________________________________________________________//
//                                                                   //

// #define OVP_COMPILATOR_GCC
// #define OVP_COMPILATOR_VisualStudio
// #define OVP_COMPILATOR_

#if defined TARGET_COMPILATOR_GCC
 #define OVP_COMPILATOR_GCC
#elif defined TARGET_COMPILATOR_VisualStudio
 #define OVP_COMPILATOR_VisualStudio
#else
 #warning "No target compilator defined !"
#endif

//___________________________________________________________________//
//                                                                   //
// API Definition                                                    //
//___________________________________________________________________//
//                                                                   //

// Taken from
// - http://people.redhat.com/drepper/dsohowto.pdf
// - http://www.nedprod.com/programs/gccvisibility.html
#if defined OVP_Shared
 #if defined OVP_OS_Windows
  #define OVP_API_Export __declspec(dllexport)
  #define OVP_API_Import __declspec(dllimport)
 #elif defined OVP_OS_Linux
  #define OVP_API_Export __attribute__((visibility("default")))
  #define OVP_API_Import __attribute__((visibility("default")))
 #else
  #define OVP_API_Export
  #define OVP_API_Import
 #endif
#else
 #define OVP_API_Export
 #define OVP_API_Import
#endif

#if defined OVP_Exports
 #define OVP_API OVP_API_Export
#else
 #define OVP_API OVP_API_Import
#endif

//___________________________________________________________________//
//                                                                   //
// NULL Definition                                                   //
//___________________________________________________________________//
//                                                                   //

#ifndef NULL
#define NULL 0
#endif

#endif // __OpenViBEPlugins_Defines_H__
