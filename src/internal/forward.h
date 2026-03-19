#ifndef XPK_INTERNAL_FORWARD_H
#define XPK_INTERNAL_FORWARD_H

struct xpkStorage;
struct xpkWriteQueue;

static inline int procXpkWriteQueueCount(xpkObject objXpk);
static inline void procXpkUnitWriteQueue(xpkObject objXpk);
static inline void procXpkMarkClean(xpkObject objXpk);
static inline void procXpkInitHead(xpkObject objXpk);
static inline void procXpkMarkAppliedLayout(xpkObject objXpk);
static inline int procXpkAddDataEntry(xpkObject objXpk, xpkEntry* pEntrySeed, const void* pData, uint64_t iSize, const xpkWriteOptions* pOpt, uint32_t* pPosRet);

#endif
