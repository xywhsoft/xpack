#ifndef _INC_RESOURCE_H
#define _INC_RESOURCE_H

#define IDC_STATIC              (-1)

#define IDR_MAINMENU            100
#define ID_FILE_NEW             101
#define ID_FILE_OPEN            102
#define ID_FILE_CLOSE           103
#define ID_FILE_EXIT            104
#define ID_FILE_ADD             105
#define ID_FILE_EXTRACT         106
#define ID_FILE_DELETE          107
#define ID_FILE_RENAME          108
#define ID_FILE_REBUILD         109
#define ID_FILE_SAVE            110
#define ID_FILE_PROPERTIES      111

#define ID_VIEW_REFRESH         200
#define ID_VIEW_LARGEICON       201
#define ID_VIEW_DETAILS         202

#define ID_TOOLS_VERIFY         300
#define ID_TOOLS_EXTRACTALL     301
#define ID_TOOLS_COMPRESS       302
#define ID_TOOLS_SOLIDMODE      303

#define ID_HELP_ABOUT           400

#define IDC_FILELIST            1000
#define IDC_STATUSBAR           1001

#define WM_ADDFILES             (WM_USER + 1)
#define WM_EXTRACTFILES         (WM_USER + 2)
#define WM_COMPRESS             (WM_USER + 3)
#define WM_REFRESH              (WM_USER + 4)

#endif
