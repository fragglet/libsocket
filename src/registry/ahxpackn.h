/* ahxpackn.h - Reverts packing alignment to default value for different
 * compilers. This file includes the appropriate header for a compiler.
 * Cannot block with #ifdefs because some compilers always process lines
 * wich start with # tokens.
 *
 * So if you want to add your compiler's specific stuff to deal with
 * packing, DO NOT add those tokens in this file. Rather add an
 * #include.
 *
 * By Alfons Hoogervorst / Alliterated Software Development, 1998.
 */

#ifdef __PACIFIC__
    /* This is for __PACIFIC__. By default __PACIFIC__ already packs
     * structure members to byte boundaries, so the following include
     * does nothing. */
#   include <_packn.pcc>
#else
    /* This is for all compilers that understand a #pragma pack() */
#   include <_packn.xcc>
#endif /* A compiler */
