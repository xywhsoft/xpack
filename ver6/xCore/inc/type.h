


// 逻辑转字符串
astr BoolToString(int bVal, int iTpe)
{
	if ( bVal ) {
		if ( iTpe == 1 ) {
			return "yes";
		} else if ( iTpe == 2 ) {
			return "on";
		} else if ( iTpe == 3 ) {
			return "是";
		} else if ( iTpe == 4 ) {
			return "对";
		} else if ( iTpe == 5 ) {
			return "真";
		} else if ( iTpe == 6 ) {
			return "允许";
		} else if ( iTpe == 7 ) {
			return "成功";
		} else {
			return "true";
		}
	} else {
		if ( iTpe == 1 ) {
			return "no";
		} else if ( iTpe == 2 ) {
			return "off";
		} else if ( iTpe == 3 ) {
			return "否";
		} else if ( iTpe == 4 ) {
			return "错";
		} else if ( iTpe == 5 ) {
			return "假";
		} else if ( iTpe == 6 ) {
			return "禁止";
		} else if ( iTpe == 7 ) {
			return "失败";
		} else {
			return "false";
		}
	}
}

// 字符串转逻辑
int StringToBool(astr sVal)
{
	if ( sVal == NULL ) {
		return FALSE;
	}
	if ( stricmp(sVal, "true") == 0 ) {
		return TRUE;
	} else if ( stricmp(sVal, "on") == 0 ) {
		return TRUE;
	} else if ( stricmp(sVal, "yes") == 0 ) {
		return TRUE;
	} else if ( strcmp(sVal, "是") == 0 ) {
		return TRUE;
	} else if ( strcmp(sVal, "对") == 0 ) {
		return TRUE;
	} else if ( strcmp(sVal, "真") == 0 ) {
		return TRUE;
	} else if ( strcmp(sVal, "允许") == 0 ) {
		return TRUE;
	} else if ( strcmp(sVal, "成功") == 0 ) {
		return TRUE;
	} else {
		return FALSE;
	}
}


