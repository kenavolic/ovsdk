#ifndef __OpenViBEPlugins_Defines_H__
#define __OpenViBEPlugins_Defines_H__

//___________________________________________________________________//
//                                                                   //
// Plugin Object Descriptor Class Identifiers                        //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_GenericNetworkAcquisitionDesc           OpenViBE::CIdentifier(0x390236CA, 0x3255E115)

/*
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00005CD6, 0x44BB8EC6)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00252EC4, 0x61B997B4)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0063C7D9, 0x0835569F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00C8C3E1, 0x6F1ED9F4)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00215579, 0x47AD9334)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x006330A8, 0x4EAA2EA4)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00318E1E, 0x3A98878A)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00CEB0FA, 0x387FAD4F)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00655DB9, 0x1CAB66F2)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x009DE4B8, 0x69323E5D)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00B0BB97, 0x54465FCE)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00F64174, 0x2508119A)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0056BF68, 0x1C10AA36)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00942C9F, 0x7258005E)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0034AC8A, 0x5F1D190B)
*/

//___________________________________________________________________//
//                                                                   //
// Plugin Object Class Identifiers                                   //
//___________________________________________________________________//
//                                                                   //

#define OVP_ClassId_GenericNetworkAcquisition               OpenViBE::CIdentifier(0x5D5CA50E, 0x7486E783)

/*
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00688668, 0x0A405170)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x004897BE, 0x6438307A)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x005F9419, 0x3F25D46A)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00DB70F3, 0x4E56D8DB)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x004B7CF3, 0x3EA0D8CC)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x007CACF9, 0x72A9CAC3)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00F35DD9, 0x03330C84)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x000BD8A9, 0x7F5D9AC5)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00BCACFF, 0x50D30982)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x006ABF8B, 0x0CA1CD9B)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00A3E1EB, 0x006D59B6)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x0000988E, 0x2F9EC0C5)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00746EF8, 0x7E4CE0E2)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00F2BA91, 0x057FEBA9)
#define OVP_ClassId_                                        OpenViBE::CIdentifier(0x00B9CB21, 0x18F61948)
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
