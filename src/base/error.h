#ifndef XPK_BASE_ERROR_H
#define XPK_BASE_ERROR_H

struct xpkErrorState {
	int iCode;
	char sText[XPK_ERROR_TEXT_CAP];
};

#endif
