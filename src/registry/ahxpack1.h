/* ahxpack1.h - Packing alignment set to 1 for different compilers.
 * This file includes the appropriate header for a compiler to pack
 * a file's structure's members. Cannot block with #ifdefs because
 * some compilers always process lines wich start with # tokens.
 *
 * So if you want to add your compiler's specific stuff to deal with
 * packing, DO NOT add those tokens in this file. Rather add an
 * #include.
 *
 * Revert to default packing by including <ahxpackn.h>.
 *
 * By Alfons Hoogervorst / Alliterated Software Development, 1998.
 */

#ifdef __PACIFIC__
    /* This is for __PACIFIC__. By default __PACIFIC__ already packs
     * structure members to byte boundaries, so the following include
     * does nothing. */
#   include <_pack1.pcc>
#else
    /* This is for all compilers that understand a #pragma pack(1) */
#   include <_pack1.xcc>
#endif /* A compiler */
