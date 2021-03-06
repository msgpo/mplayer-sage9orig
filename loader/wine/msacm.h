/*
 *      msacm.h   -       Declarations for MSACM
 */

#ifndef __WINE_MSACM_H
#define __WINE_MSACM_H

#include "windef.h"
#include "driver.h"
#include "mmreg.h"

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

//typedef WORD    VERSION;        /* major (high byte), minor (low byte) */

typedef UINT16	MMVERSION16;
typedef UINT	MMVERSION;
typedef UINT16	MCIDEVICEID16;
typedef UINT	MCIDEVICEID;
typedef	UINT16	MMRESULT16;
typedef	UINT	MMRESULT;
typedef DWORD           FOURCC;         /* a four character code */



#if !defined(WAVE_FORMAT_PCM)
#define WAVE_FORMAT_PCM     1
#endif

/***********************************************************************
 * Defines/Enums
 */

#define ACMERR_BASE        512
#define ACMERR_NOTPOSSIBLE (ACMERR_BASE + 0)
#define ACMERR_BUSY        (ACMERR_BASE + 1)
#define ACMERR_UNPREPARED  (ACMERR_BASE + 2)
#define ACMERR_CANCELED    (ACMERR_BASE + 3)

#define MM_ACM_OPEN  MM_STREAM_OPEN
#define MM_ACM_CLOSE MM_STREAM_CLOSE
#define MM_ACM_DONE  MM_STREAM_DONE

#define ACM_DRIVERADDF_FUNCTION   0x00000003L
#define ACM_DRIVERADDF_NOTIFYHWND 0x00000004L
#define ACM_DRIVERADDF_TYPEMASK   0x00000007L
#define ACM_DRIVERADDF_LOCAL      0x00000000L
#define ACM_DRIVERADDF_GLOBAL     0x00000008L

#define ACMDRIVERDETAILS_SHORTNAME_CHARS  32
#define ACMDRIVERDETAILS_LONGNAME_CHARS  128
#define ACMDRIVERDETAILS_COPYRIGHT_CHARS  80
#define ACMDRIVERDETAILS_LICENSING_CHARS 128
#define ACMDRIVERDETAILS_FEATURES_CHARS  512

#define ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC mmioFOURCC('a', 'u', 'd', 'c')
#define ACMDRIVERDETAILS_FCCCOMP_UNDEFINED  mmioFOURCC('\0', '\0', '\0', '\0')

#define ACMDRIVERDETAILS_SUPPORTF_CODEC     0x00000001L
#define ACMDRIVERDETAILS_SUPPORTF_CONVERTER 0x00000002L
#define ACMDRIVERDETAILS_SUPPORTF_FILTER    0x00000004L
#define ACMDRIVERDETAILS_SUPPORTF_HARDWARE  0x00000008L
#define ACMDRIVERDETAILS_SUPPORTF_ASYNC     0x00000010L
#define ACMDRIVERDETAILS_SUPPORTF_LOCAL     0x40000000L
#define ACMDRIVERDETAILS_SUPPORTF_DISABLED  0x80000000L

#define ACM_DRIVERENUMF_NOLOCAL  0x40000000L
#define ACM_DRIVERENUMF_DISABLED 0x80000000L

#define ACM_DRIVERPRIORITYF_ENABLE    0x00000001L
#define ACM_DRIVERPRIORITYF_DISABLE   0x00000002L
#define ACM_DRIVERPRIORITYF_ABLEMASK  0x00000003L
#define ACM_DRIVERPRIORITYF_BEGIN     0x00010000L
#define ACM_DRIVERPRIORITYF_END       0x00020000L
#define ACM_DRIVERPRIORITYF_DEFERMASK 0x00030000L

#define MM_ACM_FILTERCHOOSE 0x8000

#define FILTERCHOOSE_MESSAGE          0
#define FILTERCHOOSE_FILTERTAG_VERIFY (FILTERCHOOSE_MESSAGE+0)
#define FILTERCHOOSE_FILTER_VERIFY    (FILTERCHOOSE_MESSAGE+1)
#define FILTERCHOOSE_CUSTOM_VERIFY    (FILTERCHOOSE_MESSAGE+2)

#define ACMFILTERCHOOSE_STYLEF_SHOWHELP             0x00000004L
#define ACMFILTERCHOOSE_STYLEF_ENABLEHOOK           0x00000008L
#define ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATE       0x00000010L
#define ACMFILTERCHOOSE_STYLEF_ENABLETEMPLATEHANDLE 0x00000020L
#define ACMFILTERCHOOSE_STYLEF_INITTOFILTERSTRUCT   0x00000040L
#define ACMFILTERCHOOSE_STYLEF_CONTEXTHELP          0x00000080L

#define ACMFILTERDETAILS_FILTER_CHARS 128

#define ACM_FILTERDETAILSF_INDEX     0x00000000L
#define ACM_FILTERDETAILSF_FILTER    0x00000001L
#define ACM_FILTERDETAILSF_QUERYMASK 0x0000000FL

#define ACMFILTERTAGDETAILS_FILTERTAG_CHARS 48

#define ACM_FILTERTAGDETAILSF_INDEX       0x00000000L
#define ACM_FILTERTAGDETAILSF_FILTERTAG   0x00000001L
#define ACM_FILTERTAGDETAILSF_LARGESTSIZE 0x00000002L
#define ACM_FILTERTAGDETAILSF_QUERYMASK   0x0000000FL

#define ACM_FILTERENUMF_DWFILTERTAG 0x00010000L

#define ACMHELPMSGSTRINGA       "acmchoose_help"
#define ACMHELPMSGSTRINGW       L"acmchoose_help"
#define ACMHELPMSGSTRING16       "acmchoose_help"

#define ACMHELPMSGCONTEXTMENUA  "acmchoose_contextmenu"
#define ACMHELPMSGCONTEXTMENUW  L"acmchoose_contextmenu"
#define ACMHELPMSGCONTEXTMENU16   "acmchoose_contextmenu"

#define ACMHELPMSGCONTEXTHELPA  "acmchoose_contexthelp"
#define ACMHELPMSGCONTEXTHELPW  L"acmchoose_contexthelp"
#define ACMHELPMSGCONTEXTHELP16   "acmchoose_contexthelp"

#define MM_ACM_FORMATCHOOSE 0x8000

#define FORMATCHOOSE_MESSAGE          0
#define FORMATCHOOSE_FORMATTAG_VERIFY (FORMATCHOOSE_MESSAGE+0)
#define FORMATCHOOSE_FORMAT_VERIFY    (FORMATCHOOSE_MESSAGE+1)
#define FORMATCHOOSE_CUSTOM_VERIFY    (FORMATCHOOSE_MESSAGE+2)

#define ACMFORMATCHOOSE_STYLEF_SHOWHELP             0x00000004L
#define ACMFORMATCHOOSE_STYLEF_ENABLEHOOK           0x00000008L
#define ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATE       0x00000010L
#define ACMFORMATCHOOSE_STYLEF_ENABLETEMPLATEHANDLE 0x00000020L
#define ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT      0x00000040L
#define ACMFORMATCHOOSE_STYLEF_CONTEXTHELP          0x00000080L

#define ACMFORMATDETAILS_FORMAT_CHARS   128

#define ACM_FORMATDETAILSF_INDEX     0x00000000L
#define ACM_FORMATDETAILSF_FORMAT    0x00000001L
#define ACM_FORMATDETAILSF_QUERYMASK 0x0000000FL

#define ACM_FORMATENUMF_WFORMATTAG     0x00010000L
#define ACM_FORMATENUMF_NCHANNELS      0x00020000L
#define ACM_FORMATENUMF_NSAMPLESPERSEC 0x00040000L
#define ACM_FORMATENUMF_WBITSPERSAMPLE 0x00080000L
#define ACM_FORMATENUMF_CONVERT        0x00100000L
#define ACM_FORMATENUMF_SUGGEST        0x00200000L
#define ACM_FORMATENUMF_HARDWARE       0x00400000L
#define ACM_FORMATENUMF_INPUT          0x00800000L
#define ACM_FORMATENUMF_OUTPUT         0x01000000L

#define ACM_FORMATSUGGESTF_WFORMATTAG     0x00010000L
#define ACM_FORMATSUGGESTF_NCHANNELS      0x00020000L
#define ACM_FORMATSUGGESTF_NSAMPLESPERSEC 0x00040000L
#define ACM_FORMATSUGGESTF_WBITSPERSAMPLE 0x00080000L
#define ACM_FORMATSUGGESTF_TYPEMASK       0x00FF0000L

#define ACMFORMATTAGDETAILS_FORMATTAG_CHARS 48

#define ACM_FORMATTAGDETAILSF_INDEX       0x00000000L
#define ACM_FORMATTAGDETAILSF_FORMATTAG   0x00000001L
#define ACM_FORMATTAGDETAILSF_LARGESTSIZE 0x00000002L
#define ACM_FORMATTAGDETAILSF_QUERYMASK   0x0000000FL

#define ACM_METRIC_COUNT_DRIVERS            1
#define ACM_METRIC_COUNT_CODECS             2
#define ACM_METRIC_COUNT_CONVERTERS         3
#define ACM_METRIC_COUNT_FILTERS            4
#define ACM_METRIC_COUNT_DISABLED           5
#define ACM_METRIC_COUNT_HARDWARE           6
#define ACM_METRIC_COUNT_LOCAL_DRIVERS     20
#define ACM_METRIC_COUNT_LOCAL_CODECS      21
#define ACM_METRIC_COUNT_LOCAL_CONVERTERS  22
#define ACM_METRIC_COUNT_LOCAL_FILTERS     23
#define ACM_METRIC_COUNT_LOCAL_DISABLED    24
#define ACM_METRIC_HARDWARE_WAVE_INPUT     30
#define ACM_METRIC_HARDWARE_WAVE_OUTPUT    31
#define ACM_METRIC_MAX_SIZE_FORMAT         50
#define ACM_METRIC_MAX_SIZE_FILTER         51
#define ACM_METRIC_DRIVER_SUPPORT         100
#define ACM_METRIC_DRIVER_PRIORITY        101

#define ACM_STREAMCONVERTF_BLOCKALIGN 0x00000004
#define ACM_STREAMCONVERTF_START      0x00000010
#define ACM_STREAMCONVERTF_END        0x00000020

#define ACMSTREAMHEADER_STATUSF_DONE     0x00010000L
#define ACMSTREAMHEADER_STATUSF_PREPARED 0x00020000L
#define ACMSTREAMHEADER_STATUSF_INQUEUE  0x00100000L

#define ACM_STREAMOPENF_QUERY       0x00000001
#define ACM_STREAMOPENF_ASYNC       0x00000002
#define ACM_STREAMOPENF_NONREALTIME 0x00000004

#define ACM_STREAMSIZEF_SOURCE      0x00000000L
#define ACM_STREAMSIZEF_DESTINATION 0x00000001L
#define ACM_STREAMSIZEF_QUERYMASK   0x0000000FL

#define ACMDM_USER                  (DRV_USER + 0x0000)
#define ACMDM_RESERVED_LOW          (DRV_USER + 0x2000)
#define ACMDM_RESERVED_HIGH         (DRV_USER + 0x2FFF)

#define ACMDM_BASE                  ACMDM_RESERVED_LOW

#define ACMDM_DRIVER_ABOUT          (ACMDM_BASE + 11)

/***********************************************************************
 * Callbacks
 */

typedef WIN_BOOL CALLBACK ( *ACMDRIVERENUMCB)(
  HACMDRIVERID hadid, DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL16 CALLBACK ( *ACMDRIVERENUMCB16)(
  HACMDRIVERID16 hadid, DWORD dwInstance, DWORD fdwSupport
);

typedef UINT CALLBACK ( *ACMFILTERCHOOSEHOOKPROCA)(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);

typedef UINT CALLBACK ( *ACMFILTERCHOOSEHOOKPROCW)(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);

typedef UINT CALLBACK ( *ACMFILTERCHOOSEHOOKPROC16)(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);

typedef UINT CALLBACK ( *ACMFORMATCHOOSEHOOKPROCA)(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);

typedef UINT CALLBACK ( *ACMFORMATCHOOSEHOOKPROCW)(
  HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam
);

typedef UINT16 CALLBACK ( *ACMFORMATCHOOSEHOOKPROC16)(
  HWND16 hwnd, UINT16 uMsg, WPARAM16 wParam, LPARAM lParam
);

/***********************************************************************
 * Structures
 */

typedef struct _ACMDRIVERDETAILSA
{
  DWORD    cbStruct;

  FOURCC   fccType;
  FOURCC   fccComp;

  WORD     wMid;
  WORD     wPid;

  DWORD    vdwACM;
  DWORD    vdwDriver;

  DWORD    fdwSupport;
  DWORD    cFormatTags;
  DWORD    cFilterTags;

  HICON  hicon;

  CHAR     szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
  CHAR     szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
  CHAR     szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
  CHAR     szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
  CHAR     szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];
} ACMDRIVERDETAILSA, *PACMDRIVERDETAILSA;

typedef struct _ACMDRIVERDETAILSW
{
  DWORD    cbStruct;

  FOURCC   fccType;
  FOURCC   fccComp;

  WORD     wMid;
  WORD     wPid;

  DWORD    vdwACM;
  DWORD    vdwDriver;

  DWORD    fdwSupport;
  DWORD    cFormatTags;
  DWORD    cFilterTags;

  HICON  hicon;

  WCHAR    szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
  WCHAR    szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
  WCHAR    szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
  WCHAR    szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
  WCHAR    szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];
} ACMDRIVERDETAILSW, *PACMDRIVERDETAILSW;

typedef struct _ACMDRIVERDETAILS16
{
  DWORD   cbStruct;

  FOURCC  fccType;
  FOURCC  fccComp;

  WORD    wMid;
  WORD    wPid;

  DWORD   vdwACM;
  DWORD   vdwDriver;

  DWORD   fdwSupport;
  DWORD   cFormatTags;
  DWORD   cFilterTags;

  HICON16 hicon;

  CHAR    szShortName[ACMDRIVERDETAILS_SHORTNAME_CHARS];
  CHAR    szLongName[ACMDRIVERDETAILS_LONGNAME_CHARS];
  CHAR    szCopyright[ACMDRIVERDETAILS_COPYRIGHT_CHARS];
  CHAR    szLicensing[ACMDRIVERDETAILS_LICENSING_CHARS];
  CHAR    szFeatures[ACMDRIVERDETAILS_FEATURES_CHARS];
} ACMDRIVERDETAILS16, *NPACMDRIVERDETAILS16, *LPACMDRIVERDETAILS16;

typedef struct _ACMFILTERCHOOSEA
{
  DWORD         cbStruct;
  DWORD         fdwStyle;

  HWND        hwndOwner;

  PWAVEFILTER   pwfltr;
  DWORD         cbwfltr;

  LPCSTR        pszTitle;

  CHAR          szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
  CHAR          szFilter[ACMFILTERDETAILS_FILTER_CHARS];
  LPSTR         pszName;
  DWORD         cchName;

  DWORD         fdwEnum;
  PWAVEFILTER   pwfltrEnum;
    
  HINSTANCE   hInstance;
  LPCSTR        pszTemplateName;
  LPARAM        lCustData;
  ACMFILTERCHOOSEHOOKPROCA pfnHook;
} ACMFILTERCHOOSEA, *PACMFILTERCHOOSEA;

typedef struct _ACMFILTERCHOOSEW
{
  DWORD         cbStruct;
  DWORD         fdwStyle;

  HWND        hwndOwner;

  PWAVEFILTER   pwfltr;
  DWORD         cbwfltr;

  LPCWSTR       pszTitle;

  WCHAR         szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
  WCHAR         szFilter[ACMFILTERDETAILS_FILTER_CHARS];
  LPWSTR        pszName;
  DWORD         cchName;

  DWORD         fdwEnum;
  PWAVEFILTER   pwfltrEnum;
    
  HINSTANCE   hInstance;
  LPCWSTR       pszTemplateName;
  LPARAM        lCustData;
  ACMFILTERCHOOSEHOOKPROCW pfnHook;
} ACMFILTERCHOOSEW, *PACMFILTERCHOOSEW;

typedef struct _ACMFILTERCHOOSE16
{
  DWORD          cbStruct;
  DWORD          fdwStyle;

  HWND16         hwndOwner;

  LPWAVEFILTER   pwfltr;
  DWORD          cbwfltr;

  LPCSTR         pszTitle;

  char           szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
  char           szFilter[ACMFILTERDETAILS_FILTER_CHARS];
  LPSTR          pszName;
  DWORD          cchName;

  DWORD          fdwEnum;
  LPWAVEFILTER   pwfltrEnum;
    
  HINSTANCE16    hInstance;
  LPCSTR         pszTemplateName;
  LPARAM         lCustData;
  ACMFILTERCHOOSEHOOKPROC16 pfnHook;
} ACMFILTERCHOOSE16, *NPACMFILTERCHOOSE16, *LPACMFILTERCHOOSE16;

typedef struct _ACMFILTERDETAILSA
{
  DWORD           cbStruct;
  DWORD           dwFilterIndex;
  DWORD           dwFilterTag;
  DWORD           fdwSupport;
  PWAVEFILTER     pwfltr;
  DWORD           cbwfltr;
  CHAR            szFilter[ACMFILTERDETAILS_FILTER_CHARS];
} ACMFILTERDETAILSA, *PACMFILTERDETAILSA;

typedef struct _ACMFILTERDETAILSW
{
  DWORD          cbStruct;
  DWORD          dwFilterIndex;
  DWORD          dwFilterTag;
  DWORD          fdwSupport;
  PWAVEFILTER    pwfltr;
  DWORD          cbwfltr;
  WCHAR          szFilter[ACMFILTERDETAILS_FILTER_CHARS];
} ACMFILTERDETAILSW, *PACMFILTERDETAILSW;

typedef struct _ACMFILTERDETAILS16
{
  DWORD          cbStruct;
  DWORD          dwFilterIndex;
  DWORD          dwFilterTag;
  DWORD          fdwSupport;
  LPWAVEFILTER   pwfltr;
  DWORD          cbwfltr;
  CHAR           szFilter[ACMFILTERDETAILS_FILTER_CHARS];
} ACMFILTERDETAILS16, *NPACMFILTERDETAILS16, *LPACMFILTERDETAILS16;

typedef struct _ACMFILTERTAGDETAILSA
{
  DWORD cbStruct;
  DWORD dwFilterTagIndex;
  DWORD dwFilterTag;
  DWORD cbFilterSize;
  DWORD fdwSupport;
  DWORD cStandardFilters;
  CHAR  szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
} ACMFILTERTAGDETAILSA, *PACMFILTERTAGDETAILSA;

typedef struct _ACMFILTERTAGDETAILSW
{
  DWORD cbStruct;
  DWORD dwFilterTagIndex;
  DWORD dwFilterTag;
  DWORD cbFilterSize;
  DWORD fdwSupport;
  DWORD cStandardFilters;
  WCHAR szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
} ACMFILTERTAGDETAILSW, *PACMFILTERTAGDETAILSW;

typedef struct _ACMFILTERTAGDETAILS16
{
  DWORD cbStruct;
  DWORD dwFilterTagIndex;
  DWORD dwFilterTag;
  DWORD cbFilterSize;
  DWORD fdwSupport;
  DWORD cStandardFilters;
  CHAR  szFilterTag[ACMFILTERTAGDETAILS_FILTERTAG_CHARS];
} ACMFILTERTAGDETAILS16, *NPACMFILTERTAGDETAILS16, *LPACMFILTERTAGDETAILS16;

typedef struct _ACMFORMATCHOOSEA
{
  DWORD           cbStruct;
  DWORD           fdwStyle;
 
  HWND          hwndOwner;

  PWAVEFORMATEX   pwfx;
  DWORD           cbwfx;
  LPCSTR          pszTitle;
    
  CHAR            szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
  CHAR            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

  LPSTR           pszName;
  DWORD           cchName;

  DWORD           fdwEnum;
  PWAVEFORMATEX   pwfxEnum;
    
  HINSTANCE     hInstance;
  LPCSTR          pszTemplateName;
  LPARAM          lCustData;
  ACMFORMATCHOOSEHOOKPROCA pfnHook;
} ACMFORMATCHOOSEA, *PACMFORMATCHOOSEA;

typedef struct _ACMFORMATCHOOSEW
{
  DWORD           cbStruct;
  DWORD           fdwStyle;
    
  HWND          hwndOwner;

  PWAVEFORMATEX   pwfx;
  DWORD           cbwfx;
  LPCWSTR         pszTitle;
    
  WCHAR           szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
  WCHAR           szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

  LPWSTR          pszName;
  DWORD           cchName;

  DWORD           fdwEnum;
  LPWAVEFORMATEX  pwfxEnum;
    
  HINSTANCE     hInstance;
  LPCWSTR         pszTemplateName;
  LPARAM          lCustData;
  ACMFORMATCHOOSEHOOKPROCW pfnHook;
} ACMFORMATCHOOSEW, *PACMFORMATCHOOSEW;

typedef struct _ACMFORMATCHOOSE16
{
  DWORD            cbStruct;
  DWORD            fdwStyle;
    
  HWND16           hwndOwner;

  LPWAVEFORMATEX   pwfx;
  DWORD            cbwfx;
  LPCSTR           pszTitle;
    
  CHAR             szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
  CHAR             szFormat[ACMFORMATDETAILS_FORMAT_CHARS];    

  LPSTR            pszName;
  DWORD            cchName;

  DWORD            fdwEnum;
  LPWAVEFORMATEX   pwfxEnum;
    
  HINSTANCE16      hInstance;
  LPCSTR           pszTemplateName;
  LPARAM           lCustData;
  ACMFORMATCHOOSEHOOKPROC16 pfnHook;
} ACMFORMATCHOOSE16, *NPACMFORMATCHOOSE16, *LPACMFORMATCHOOSE16;

typedef struct _ACMFORMATDETAILSA
{
  DWORD           cbStruct;
  DWORD           dwFormatIndex;
  DWORD           dwFormatTag;
  DWORD           fdwSupport;
  PWAVEFORMATEX   pwfx;
  DWORD           cbwfx;
  CHAR            szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
} ACMFORMATDETAILSA, *PACMFORMATDETAILSA;

typedef struct _ACMFORMATDETAILSW
{
    DWORD           cbStruct;
    DWORD           dwFormatIndex;
    DWORD           dwFormatTag;
    DWORD           fdwSupport;
    PWAVEFORMATEX   pwfx;
    DWORD           cbwfx;
    WCHAR           szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
} ACMFORMATDETAILSW, *PACMFORMATDETAILSW;

typedef struct _ACMFORMATDETAILS16
{
    DWORD            cbStruct;
    DWORD            dwFormatIndex;
    DWORD            dwFormatTag;
    DWORD            fdwSupport;
    LPWAVEFORMATEX   pwfx;
    DWORD            cbwfx;
    CHAR             szFormat[ACMFORMATDETAILS_FORMAT_CHARS];
} ACMFORMATDETAILS16, *NPACMFORMATDETAILS16, *LPACMFORMATDETAILS16;

typedef struct _ACMFORMATTAGDETAILSA
{
  DWORD cbStruct;
  DWORD dwFormatTagIndex;
  DWORD dwFormatTag;
  DWORD cbFormatSize;
  DWORD fdwSupport;
  DWORD cStandardFormats;
  CHAR  szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
} ACMFORMATTAGDETAILSA, *PACMFORMATTAGDETAILSA;

typedef struct _ACMFORMATTAGDETAILSW
{
  DWORD cbStruct;
  DWORD dwFormatTagIndex;
  DWORD dwFormatTag;
  DWORD cbFormatSize;
  DWORD fdwSupport;
  DWORD cStandardFormats;
  WCHAR szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
} ACMFORMATTAGDETAILSW, *PACMFORMATTAGDETAILSW;

typedef struct _ACMFORMATTAGDETAILS16
{
  DWORD cbStruct;
  DWORD dwFormatTagIndex;
  DWORD dwFormatTag;
  DWORD cbFormatSize;
  DWORD fdwSupport;
  DWORD cStandardFormats;
  CHAR  szFormatTag[ACMFORMATTAGDETAILS_FORMATTAG_CHARS];
} ACMFORMATTAGDETAILS16, *NPACMFORMATTAGDETAILS16, *LPACMFORMATTAGDETAILS16;

typedef struct _ACMSTREAMHEADER
{
  DWORD  cbStruct;
  DWORD  fdwStatus;
  DWORD  dwUser;
  LPBYTE pbSrc;
  DWORD  cbSrcLength;
  DWORD  cbSrcLengthUsed;
  DWORD  dwSrcUser;
  LPBYTE pbDst;
  DWORD  cbDstLength;
  DWORD  cbDstLengthUsed;
  DWORD  dwDstUser;
  DWORD  dwReservedDriver[10];
} ACMSTREAMHEADER16, *NPACMSTREAMHEADER16, *LPACMSTREAMHEADER16,
  ACMSTREAMHEADER, *PACMSTREAMHEADER;

/***********************************************************************
 * Callbacks 2
 */

typedef WIN_BOOL CALLBACK ( *ACMFILTERENUMCBA)(
  HACMDRIVERID hadid, PACMFILTERDETAILSA pafd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFILTERENUMCBW)(
  HACMDRIVERID hadid, PACMFILTERDETAILSW pafd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL16 CALLBACK ( *ACMFILTERENUMCB16)(
 HACMDRIVERID16 hadid, LPACMFILTERDETAILS16 pafd,
 DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFILTERTAGENUMCBA)(
  HACMDRIVERID hadid, PACMFILTERTAGDETAILSA paftd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFILTERTAGENUMCBW)(
  HACMDRIVERID hadid, PACMFILTERTAGDETAILSW paftd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL16 CALLBACK ( *ACMFILTERTAGENUMCB16)(
  HACMDRIVERID16 hadid, LPACMFILTERTAGDETAILS16 paftd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFORMATENUMCBA)(
  HACMDRIVERID hadid, PACMFORMATDETAILSA pafd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFORMATENUMCBW)(
  HACMDRIVERID hadid, PACMFORMATDETAILSW pafd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL16 CALLBACK ( *ACMFORMATENUMCB16)(
  HACMDRIVERID16 hadid, LPACMFORMATDETAILS16 pafd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFORMATTAGENUMCBA)(
  HACMDRIVERID hadid, PACMFORMATTAGDETAILSA paftd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL CALLBACK ( *ACMFORMATTAGENUMCBW)(
  HACMDRIVERID hadid, PACMFORMATTAGDETAILSW paftd,
  DWORD dwInstance, DWORD fdwSupport
);

typedef WIN_BOOL16 CALLBACK ( *ACMFORMATTAGENUMCB16)(
  HACMDRIVERID16 hadid, LPACMFORMATTAGDETAILS16 paftd,
  DWORD dwInstance, DWORD fdwSupport
);

/***********************************************************************
 * Functions - Win16
 */

DWORD WINAPI acmGetVersion16(
);
MMRESULT16 WINAPI acmMetrics16(
  HACMOBJ16 hao, UINT16 uMetric, LPVOID pMetric
);
MMRESULT16 WINAPI acmDriverEnum16(
  ACMDRIVERENUMCB16 fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT16 WINAPI acmDriverDetails16(
  HACMDRIVERID16 hadid, LPACMDRIVERDETAILS16 padd, DWORD fdwDetails
);
MMRESULT16 WINAPI acmDriverAdd16(
  LPHACMDRIVERID16 phadid, HINSTANCE16 hinstModule,
  LPARAM lParam, DWORD dwPriority, DWORD fdwAdd
);
MMRESULT16 WINAPI acmDriverRemove16(
  HACMDRIVERID16 hadid, DWORD fdwRemove
);
MMRESULT16 WINAPI acmDriverOpen16(
  LPHACMDRIVER16 phad, HACMDRIVERID16 hadid, DWORD fdwOpen
);
MMRESULT16 WINAPI acmDriverClose16(
  HACMDRIVER16 had, DWORD fdwClose
);
LRESULT WINAPI acmDriverMessage16(
  HACMDRIVER16 had, UINT16 uMsg, LPARAM lParam1, LPARAM lParam2
);
MMRESULT16 WINAPI acmDriverID16(
  HACMOBJ16 hao, LPHACMDRIVERID16 phadid, DWORD fdwDriverID
);
MMRESULT16 WINAPI acmDriverPriority16(
 HACMDRIVERID16 hadid, DWORD dwPriority, DWORD fdwPriority
);
MMRESULT16 WINAPI acmFormatTagDetails16(
  HACMDRIVER16 had, LPACMFORMATTAGDETAILS16 paftd, DWORD fdwDetails
);
MMRESULT16 WINAPI acmFormatTagEnum16(
  HACMDRIVER16 had, LPACMFORMATTAGDETAILS16 paftd,
  ACMFORMATTAGENUMCB16 fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT16 WINAPI acmFormatChoose16(
  LPACMFORMATCHOOSE16 pafmtc
);
MMRESULT16 WINAPI acmFormatDetails16(
  HACMDRIVER16 had, LPACMFORMATDETAILS16 pafd, DWORD fdwDetails
);
MMRESULT16 WINAPI acmFormatEnum16(
  HACMDRIVER16 had, LPACMFORMATDETAILS16 pafd,
  ACMFORMATENUMCB16 fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT16 WINAPI acmFormatSuggest16(
  HACMDRIVER16 had, LPWAVEFORMATEX pwfxSrc, 
  LPWAVEFORMATEX pwfxDst, DWORD cbwfxDst, DWORD fdwSuggest
);
MMRESULT16 WINAPI acmFilterTagDetails16(
  HACMDRIVER16 had, LPACMFILTERTAGDETAILS16 paftd, DWORD fdwDetails
);
MMRESULT16 WINAPI acmFilterTagEnum16(
  HACMDRIVER16 had, LPACMFILTERTAGDETAILS16 paftd,
  ACMFILTERTAGENUMCB16 fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT16 WINAPI acmFilterChoose16(
  LPACMFILTERCHOOSE16 pafltrc
);
MMRESULT16 WINAPI acmFilterDetails16(
  HACMDRIVER16 had, LPACMFILTERDETAILS16 pafd, DWORD fdwDetails
);
MMRESULT16 WINAPI acmFilterEnum16(
  HACMDRIVER16 had, LPACMFILTERDETAILS16 pafd,
  ACMFILTERENUMCB16 fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT16 WINAPI acmStreamOpen16(
  LPHACMSTREAM16 phas, HACMDRIVER16 had,
  LPWAVEFORMATEX pwfxSrc, LPWAVEFORMATEX pwfxDst,
  LPWAVEFILTER pwfltr, DWORD dwCallback,
  DWORD dwInstance, DWORD fdwOpen
);
MMRESULT16 WINAPI acmStreamClose16(
  HACMSTREAM16 has, DWORD fdwClose
);
MMRESULT16 WINAPI acmStreamSize16(
  HACMSTREAM16 has, DWORD cbInput, 
  LPDWORD pdwOutputBytes, DWORD fdwSize
);
MMRESULT16 WINAPI acmStreamConvert16(
  HACMSTREAM16 has, LPACMSTREAMHEADER16 pash, DWORD fdwConvert
);
MMRESULT16 WINAPI acmStreamReset16(
  HACMSTREAM16 has, DWORD fdwReset
);
MMRESULT16 WINAPI acmStreamPrepareHeader16(
  HACMSTREAM16 has, LPACMSTREAMHEADER16 pash, DWORD fdwPrepare
);
MMRESULT16 WINAPI acmStreamUnprepareHeader16(
  HACMSTREAM16 has, LPACMSTREAMHEADER16 pash, DWORD fdwUnprepare
);

/***********************************************************************
 * Functions - Win32
 */

MMRESULT WINAPI acmDriverAddA(
  PHACMDRIVERID phadid, HINSTANCE hinstModule,
  LPARAM lParam, DWORD dwPriority, DWORD fdwAdd
);
MMRESULT WINAPI acmDriverAddW(
  PHACMDRIVERID phadid, HINSTANCE hinstModule,
  LPARAM lParam, DWORD dwPriority, DWORD fdwAdd
);
MMRESULT WINAPI acmDriverClose(
  HACMDRIVER had, DWORD fdwClose
);
MMRESULT WINAPI acmDriverDetailsA(
  HACMDRIVERID hadid, PACMDRIVERDETAILSA padd, DWORD fdwDetails
);
MMRESULT WINAPI acmDriverDetailsW(
  HACMDRIVERID hadid, PACMDRIVERDETAILSW padd, DWORD fdwDetails
);
MMRESULT WINAPI acmDriverEnum(
  ACMDRIVERENUMCB fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmDriverID(
  HACMOBJ hao, PHACMDRIVERID phadid, DWORD fdwDriverID
);
LRESULT WINAPI acmDriverMessage(
  HACMDRIVER had, UINT uMsg, LPARAM lParam1, LPARAM lParam2
);
MMRESULT WINAPI acmDriverOpen(
  PHACMDRIVER phad, HACMDRIVERID hadid, DWORD fdwOpen
);
MMRESULT WINAPI acmDriverPriority(
  HACMDRIVERID hadid, DWORD dwPriority, DWORD fdwPriority
);
MMRESULT WINAPI acmDriverRemove(
  HACMDRIVERID hadid, DWORD fdwRemove
);
MMRESULT WINAPI acmFilterChooseA(
  PACMFILTERCHOOSEA pafltrc
);
MMRESULT WINAPI acmFilterChooseW(
  PACMFILTERCHOOSEW pafltrc
);
MMRESULT WINAPI acmFilterDetailsA(
  HACMDRIVER had, PACMFILTERDETAILSA pafd, DWORD fdwDetails
);
MMRESULT WINAPI acmFilterDetailsW(
  HACMDRIVER had, PACMFILTERDETAILSW pafd, DWORD fdwDetails
);
MMRESULT WINAPI acmFilterEnumA(
  HACMDRIVER had, PACMFILTERDETAILSA pafd, 
  ACMFILTERENUMCBA fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFilterEnumW(
  HACMDRIVER had, PACMFILTERDETAILSW pafd, 
  ACMFILTERENUMCBW fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFilterTagDetailsA(
  HACMDRIVER had, PACMFILTERTAGDETAILSA paftd, DWORD fdwDetails
);
MMRESULT WINAPI acmFilterTagDetailsW(
  HACMDRIVER had, PACMFILTERTAGDETAILSW paftd, DWORD fdwDetails
);
MMRESULT WINAPI acmFilterTagEnumA(
  HACMDRIVER had, PACMFILTERTAGDETAILSA paftd,
  ACMFILTERTAGENUMCBA fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFilterTagEnumW(
  HACMDRIVER had, PACMFILTERTAGDETAILSW paftd,
  ACMFILTERTAGENUMCBW fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFormatChooseA(
  PACMFORMATCHOOSEA pafmtc
);
MMRESULT WINAPI acmFormatChooseW(
  PACMFORMATCHOOSEW pafmtc
);
MMRESULT WINAPI acmFormatDetailsA(
  HACMDRIVER had, PACMFORMATDETAILSA pafd, DWORD fdwDetails
);
MMRESULT WINAPI acmFormatDetailsW(
  HACMDRIVER had, PACMFORMATDETAILSW pafd, DWORD fdwDetails
);
MMRESULT WINAPI acmFormatEnumA(
  HACMDRIVER had, PACMFORMATDETAILSA pafd,
  ACMFORMATENUMCBA fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFormatEnumW(
  HACMDRIVER had, PACMFORMATDETAILSW pafd,
  ACMFORMATENUMCBW fnCallback, DWORD dwInstance,  DWORD fdwEnum
);
MMRESULT WINAPI acmFormatSuggest(
  HACMDRIVER had, PWAVEFORMATEX pwfxSrc, PWAVEFORMATEX pwfxDst,
  DWORD cbwfxDst, DWORD fdwSuggest
);
MMRESULT WINAPI acmFormatTagDetailsA(
  HACMDRIVER had, PACMFORMATTAGDETAILSA paftd, DWORD fdwDetails
);
MMRESULT WINAPI acmFormatTagDetailsW(
  HACMDRIVER had, PACMFORMATTAGDETAILSW paftd, DWORD fdwDetails
);
MMRESULT WINAPI acmFormatTagEnumA(
  HACMDRIVER had, PACMFORMATTAGDETAILSA paftd,
  ACMFORMATTAGENUMCBA fnCallback, DWORD dwInstance, DWORD fdwEnum
);
MMRESULT WINAPI acmFormatTagEnumW(
  HACMDRIVER had, PACMFORMATTAGDETAILSW paftd,
  ACMFORMATTAGENUMCBW fnCallback, DWORD dwInstance, DWORD fdwEnum
);
DWORD WINAPI acmGetVersion(
);
MMRESULT WINAPI acmMetrics(
  HACMOBJ hao, UINT  uMetric, LPVOID  pMetric
);
MMRESULT WINAPI acmStreamClose(
  HACMSTREAM has, DWORD fdwClose
);
MMRESULT WINAPI acmStreamConvert(
  HACMSTREAM has, PACMSTREAMHEADER pash, DWORD fdwConvert
);
MMRESULT WINAPI acmStreamMessage(
  HACMSTREAM has, UINT uMsg, LPARAM lParam1, LPARAM lParam2
);
MMRESULT WINAPI acmStreamOpen(
  PHACMSTREAM phas, HACMDRIVER had, PWAVEFORMATEX pwfxSrc,
  PWAVEFORMATEX pwfxDst, PWAVEFILTER pwfltr, DWORD dwCallback,
  DWORD dwInstance, DWORD fdwOpen
);
MMRESULT WINAPI acmStreamPrepareHeader(
  HACMSTREAM has, PACMSTREAMHEADER pash, DWORD fdwPrepare
);
MMRESULT WINAPI acmStreamReset(
  HACMSTREAM has, DWORD fdwReset
);
MMRESULT WINAPI acmStreamSize(
  HACMSTREAM has, DWORD cbInput, 
  LPDWORD pdwOutputBytes, DWORD fdwSize
);
MMRESULT WINAPI acmStreamUnprepareHeader(
  HACMSTREAM has, PACMSTREAMHEADER pash, DWORD fdwUnprepare
);

#ifdef __cplusplus
} /* extern "C" */
#endif /* defined(__cplusplus) */

#endif  /* __WINE_MSACM_H */


