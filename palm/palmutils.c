#include <PalmTypes.h>
#include <FeatureMgr.h>
#include <ErrorBase.h>
#include <ErrorMgr.h>
#include <SystemMgr.h>
#include <SysUtils.h>
#include <StringMgr.h>
#include <Form.h>

#include <resCompat.h>
#include <simcity_resconsts.h>

static UInt32 oWidth = 0;
static UInt32 oHeight = 0;
static UInt32 oDepth = 0;
static Boolean oUseColor = 0;
/* included the TRG magic numbers :( */
#define TRGSysFtrID             'TRG '
#define TRGVgaFtrNum            2

static Boolean
isHandEra()
{
    UInt32 version;
    if (FtrGet(TRGSysFtrID, TRGVgaFtrNum, &version) == 0)
        if (sysGetROMVerMajor(version) >= 1) return (true);
    return (false);
}

/* Return the depth in bits per pixel */
UInt32
getDepth(void)
{
    static UInt32 avd = 0;
    extern short int oldROM;
    if (avd != 0)
        return (avd);
    if (!oldROM) {
	    (void) _WinScreenMode(winScreenModeGet, NULL, NULL,
				  &avd, NULL);
	    /* avd = 1 << (avd-1); */
    } else avd = 1;
    return (avd);
}

UInt32
hibit(UInt32 x)
{
    int r = 0;
    if ( x & 0xffff0000 )  { x >>= 16;  r += 16; }
    if ( x & 0x0000ff00 )  { x >>=  8;  r +=  8; }
    if ( x & 0x000000f0 )  { x >>=  4;  r +=  4; }
    if ( x & 0x0000000c )  { x >>=  2;  r +=  2; }
    if ( x & 0x00000002 )  {            r +=  1; }
    return r;
}

Err
changeDepthRes(UInt32 ndepth)
{
    UInt32 depth = ndepth;
    Boolean enablecol = 1;
    UInt32 dep = 0;
    UInt32 cdep;
    UInt32 width;
    UInt32 height;
    Err result;

    SETWIDTH(160);
    SETHEIGHT(160);

    (void)loadHiRes();
    if (isHires()) {
        SETWIDTH(320);
        SETHEIGHT(320);
    }
    (void) _WinScreenMode(winScreenModeGet, &oWidth, &oHeight, &oDepth,
        &oUseColor);
    (void) _WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL,
			  &enablecol);
    (void) _WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL,
                          &dep, NULL);
    /* in theory there's 16color _as well as_ 16grays */
    cdep = 1 + hibit(dep);
    if ((cdep >= ndepth) && (ndepth > 1)) {
        do {
            cdep = hibit(dep);
            if ((cdep+1) & ndepth) {
                depth = cdep + 1;
                break;
            }
            dep = dep & ~(1 << cdep);
        } while (dep);

        /* Could not match... */
        if (!dep) {
            (void) unloadHiRes();
            depth = 1;
            enablecol = 0;
            SETWIDTH(160);
            SETHEIGHT(160);
        }
    } else {
      enablecol = 0;
      depth = 1;
      SETWIDTH(160);
      SETHEIGHT(160);
      (void) unloadHiRes();
    }

    width = sWidth;
    height = sHeight;

    if (isHandEra())
        result = _WinScreenMode(winScreenModeSet, NULL, NULL, &depth, &enablecol);
    else
        result = _WinScreenMode(winScreenModeSet, &width, &height, &depth,
          &enablecol);
    
    return (result);
}

Err
restoreDepthRes(void)
{
    
    UInt32 de;
    UInt32 wi;
    UInt32 he;
    Boolean ec;
    Err rv;
    de = oDepth;
    wi = oWidth;
    he = oHeight;
    ec = oUseColor;
    if (canHires())
    if ((rv = _WinScreenMode(winScreenModeSet, &wi, &he, &de, &ec)) != 0)
        return (rv);
    if (0 != (rv = unloadHiRes())) return (rv);
        return (0);
}

Boolean
canColor(UInt16 nbits)
{
    static Boolean rv = false;
    static Boolean inited = false;
    if (!inited) {
        UInt32 de;
        UInt32 wi = 160;
        UInt32 he = 160;
        Boolean ec;
        inited = true;
        WinScreenMode(winScreenModeGetSupportedDepths, &wi, &he, &de, &ec);
        if (de & (1<<(nbits-1)))
            rv = true;
    }
    return (rv);
}

UInt32
GetCreatorID(void)
{
    static UInt32 nCreatorID = 0;

    if (nCreatorID == 0) {
        UInt16 nCard;
        LocalID LocalDB;
        Err err;
        err = SysCurAppDatabase(&nCard, &LocalDB);
        ErrFatalDisplayIf(err, "Could not get current app database.");
        err = DmDatabaseInfo(nCard, LocalDB, 0, 0, 0, 0, 0, 0, 0, 0,
          0, 0, &nCreatorID);
        ErrFatalDisplayIf(err,
          "Could not get app database info, looking for creator ID");
    }

    return nCreatorID;
}

void
DangerWillRobinson(char *information, char *file, int line)
{
    char buffer[80];
    StrPrintF(buffer, "%s(%d)", file, line);
    FrmCustomAlert(alertID_programmingNiggle, information, buffer, NULL);
}

/* build a string list from all the string list items from resID */
Char **
FillStringList(UInt16 resID, UInt16 *length)
{
    UInt16 max = 0;
    UInt16 atitem = 0;
    Char *foo = NULL;
    Char **rv = NULL;
    Char *lom;
    Char item[201];
    UInt32 maxlen = 0;

    do {
        foo = SysStringByIndex(resID, max, item, 200);
        if (*foo != '\0') {
            maxlen += 1+StrLen(item);
            max++;
        } else break;
    } while (foo);
    rv = MemPtrNew(sizeof(char *) * (max+1));
    lom = MemPtrNew(sizeof(char) * maxlen);
    rv[0] = lom;
    while(atitem < max) {
        SysStringByIndex(resID, atitem, item, 200);
        StrCopy(rv[atitem], item);
        rv[atitem+1] = rv[atitem] + (StrLen(item)+1);
        atitem++;
    }
    *length = max;
    return (rv);
}

/* release from above */
void
FreeStringList(Char **list)
{
    MemPtrFree(list[0]);
    MemPtrFree(list);
}

