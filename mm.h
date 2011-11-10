/*
 * =====================================================================================
 *
 *       Filename:  mm.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  21.10.2011 10:27:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef MM_H
#define MM_H

typedef     unsigned long    frame_t;    // the number of a physical page frame
typedef     unsigned long    page_t;     // the number of a virtual page

int mm_init();




#endif // MM_H

