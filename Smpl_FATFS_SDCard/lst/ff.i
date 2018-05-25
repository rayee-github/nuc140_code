#line 1 "ff8\\src\\ff.c"



















































































 

#line 1 "ff8\\src\\ff.h"














 








#line 1 "ff8\\src\\integer.h"
 
 
 






 
typedef int				INT;
typedef unsigned int	UINT;

 
typedef char			CHAR;
typedef unsigned char	UCHAR;
typedef unsigned char	BYTE;

 
typedef short			SHORT;
typedef unsigned short	USHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

 
typedef long			LONG;
typedef unsigned long	ULONG;
typedef unsigned long	DWORD;


#line 25 "ff8\\src\\ff.h"
#line 1 "ff8\\src\\ffconf.h"







 






 




 





 









 



 



 



 



 





 































 














 




 





 





 


 







 





 





 











 





 
 






 






 


#line 26 "ff8\\src\\ff.h"






 

#line 104 "ff8\\src\\ff.h"

#line 224 "ff8\\src\\ff.h"



 

#line 241 "ff8\\src\\ff.h"





 

#line 260 "ff8\\src\\ff.h"
typedef char TCHAR;








 

#line 281 "ff8\\src\\ff.h"



 

typedef struct {
	BYTE	fs_type;		 
	BYTE	drv;			 
	BYTE	csize;			 
	BYTE	n_fats;			 
	BYTE	wflag;			 
	BYTE	fsi_flag;		 
	WORD	id;				 
	WORD	n_rootdir;		 
#line 302 "ff8\\src\\ff.h"
	DWORD	last_clust;		 
	DWORD	free_clust;		 
	DWORD	fsi_sector;		 


	DWORD	cdir;			 

	DWORD	n_fatent;		 
	DWORD	fsize;			 
	DWORD	fatbase;		 
	DWORD	dirbase;		 
	DWORD	database;		 
	DWORD	winsect;		 
	BYTE	win[512];	 



} FATFS;



 

typedef struct {
	FATFS*	fs;				 
	WORD	id;				 
	BYTE	flag;			 
	BYTE	pad1;
	DWORD	fptr;			 
	DWORD	fsize;			 
	DWORD	org_clust;		 
	DWORD	curr_clust;		 
	DWORD	dsect;			 

	DWORD	dir_sect;		 
	BYTE*	dir_ptr;		 
#line 346 "ff8\\src\\ff.h"
	BYTE	buf[512];	 

} FIL;



 

typedef struct {
	FATFS*	fs;				 
	WORD	id;				 
	WORD	index;			 
	DWORD	sclust;			 
	DWORD	clust;			 
	DWORD	sect;			 
	BYTE*	dir;			 
	BYTE*	fn;				 

	WCHAR*	lfn;			 
	WORD	lfn_idx;		 

} DIR;



 

typedef struct {
	DWORD	fsize;			 
	WORD	fdate;			 
	WORD	ftime;			 
	BYTE	fattrib;		 
	TCHAR	fname[13];		 

	TCHAR*	lfname;			 
	int 	lfsize;			 

} FILINFO;



 

typedef enum {
	FR_OK = 0,				 
	FR_DISK_ERR,			 
	FR_INT_ERR,				 
	FR_NOT_READY,			 
	FR_NO_FILE,				 
	FR_NO_PATH,				 
	FR_INVALID_NAME,		 
	FR_DENIED,				 
	FR_EXIST,				 
	FR_INVALID_OBJECT,		 
	FR_WRITE_PROTECTED,		 
	FR_INVALID_DRIVE,		 
	FR_NOT_ENABLED,			 
	FR_NO_FILESYSTEM,		 
	FR_MKFS_ABORTED,		 
	FR_TIMEOUT,				 
	FR_LOCKED,				 
	FR_NOT_ENOUGH_CORE,		 
	FR_TOO_MANY_OPEN_FILES	 
} FRESULT;



 
 

FRESULT f_mount (BYTE, FATFS*);						 
FRESULT f_open (FIL*, const TCHAR*, BYTE);			 
FRESULT f_read (FIL*, void*, UINT, UINT*);			 
FRESULT f_lseek (FIL*, DWORD);						 
FRESULT f_close (FIL*);								 
FRESULT f_opendir (DIR*, const TCHAR*);				 
FRESULT f_readdir (DIR*, FILINFO*);					 
FRESULT f_stat (const TCHAR*, FILINFO*);			 

FRESULT f_write (FIL*, const void*, UINT, UINT*);	 
FRESULT f_getfree (const TCHAR*, DWORD*, FATFS**);	 
FRESULT f_truncate (FIL*);							 
FRESULT f_sync (FIL*);								 
FRESULT f_unlink (const TCHAR*);					 
FRESULT	f_mkdir (const TCHAR*);						 
FRESULT f_chmod (const TCHAR*, BYTE, BYTE);			 
FRESULT f_utime (const TCHAR*, const FILINFO*);		 
FRESULT f_rename (const TCHAR*, const TCHAR*);		 





FRESULT f_mkfs (BYTE, BYTE, UINT);					 


FRESULT f_chdir (const TCHAR*);						 
FRESULT f_chdrive (BYTE);							 
#line 456 "ff8\\src\\ff.h"



 
 

 

DWORD get_fattime (void);


 

WCHAR ff_convert (WCHAR, UINT);		 
WCHAR ff_wtoupper (WCHAR);			 






 
#line 484 "ff8\\src\\ff.h"




 
 


 





#line 506 "ff8\\src\\ff.h"


 






 

#line 525 "ff8\\src\\ff.h"


 





 

#line 550 "ff8\\src\\ff.h"







#line 568 "ff8\\src\\ff.h"








#line 591 "ff8\\src\\ff.h"



 
 

#line 608 "ff8\\src\\ff.h"





#line 87 "ff8\\src\\ff.c"
#line 1 "ff8\\src\\diskio.h"


 






#line 11 "ff8\\src\\diskio.h"
#line 12 "ff8\\src\\diskio.h"

 
typedef BYTE	DSTATUS;

 
typedef enum {
	RES_OK = 0,		 
	RES_ERROR,		 
	RES_WRPRT,		 
	RES_NOTRDY,		 
	RES_PARERR		 
} DRESULT;


 
 

DSTATUS disk_initialize (BYTE);
DSTATUS disk_status (BYTE);
DRESULT disk_read (BYTE, BYTE*, DWORD, BYTE);

DRESULT disk_write (BYTE, const BYTE*, DWORD, BYTE);

DRESULT disk_ioctl (BYTE, BYTE, void*);
void	disk_timerproc (void);




 






 

 
#line 58 "ff8\\src\\diskio.h"
 





 





 








#line 88 "ff8\\src\\ff.c"






 






 
 




 
#line 115 "ff8\\src\\ff.c"


 
#line 128 "ff8\\src\\ff.c"






 




#line 154 "ff8\\src\\ff.c"







 
#line 169 "ff8\\src\\ff.c"



 
 




static
WORD Fsid;				 
static
FATFS *FatFs[1];	 


static
BYTE Drive;				 



#line 195 "ff8\\src\\ff.c"
static WCHAR LfnBuf[127 + 1];




#line 215 "ff8\\src\\ff.c"








 


 
 
 

 
static
void mem_cpy (void* dst, const void* src, int cnt) {
	BYTE *d = (BYTE*)dst;
	const BYTE *s = (const BYTE*)src;

#line 243 "ff8\\src\\ff.c"
	while (cnt--)
		*d++ = *s++;
}

 
static
void mem_set (void* dst, int val, int cnt) {
	BYTE *d = (BYTE*)dst;

	while (cnt--)
		*d++ = (BYTE)val;
}

 
static
int mem_cmp (const void* dst, const void* src, int cnt) {
	const BYTE *d = (const BYTE *)dst, *s = (const BYTE *)src;
	int r = 0;

	while (cnt-- && (r = *d++ - *s++) == 0) ;
	return r;
}

 
static
int chk_chr (const char* str, int chr) {
	while (*str && *str != chr) str++;
	return *str;
}



 
 
 
#line 303 "ff8\\src\\ff.c"



 
 
 
#line 401 "ff8\\src\\ff.c"



 
 
 

static
FRESULT move_window (
	FATFS *fs,		 
	DWORD sector	 
)					 
{
	DWORD wsect;


	wsect = fs->winsect;
	if (wsect != sector) {	 

		if (fs->wflag) {	 
			if (disk_write(fs->drv, fs->win, wsect, 1) != RES_OK)
				return FR_DISK_ERR;
			fs->wflag = 0;
			if (wsect < (fs->fatbase + fs->fsize)) {	 
				BYTE nf;
				for (nf = fs->n_fats; nf > 1; nf--) {	 
					wsect += fs->fsize;
					disk_write(fs->drv, fs->win, wsect, 1);
				}
			}
		}

		if (sector) {
			if (disk_read(fs->drv, fs->win, sector, 1) != RES_OK)
				return FR_DISK_ERR;
			fs->winsect = sector;
		}
	}

	return FR_OK;
}




 
 
 

static
FRESULT sync (	 
	FATFS *fs	 
)
{
	FRESULT res;


	res = move_window(fs, 0);
	if (res == FR_OK) {
		 
		if (fs->fs_type == 3 && fs->fsi_flag) {
			fs->winsect = 0;
			mem_set(fs->win, 0, 512);
			*(BYTE*)(fs->win+510)=(BYTE)(0xAA55); *(BYTE*)((fs->win+510)+1)=(BYTE)((WORD)(0xAA55)>>8);
			*(BYTE*)(fs->win+0)=(BYTE)(0x41615252); *(BYTE*)((fs->win+0)+1)=(BYTE)((WORD)(0x41615252)>>8); *(BYTE*)((fs->win+0)+2)=(BYTE)((DWORD)(0x41615252)>>16); *(BYTE*)((fs->win+0)+3)=(BYTE)((DWORD)(0x41615252)>>24);
			*(BYTE*)(fs->win+484)=(BYTE)(0x61417272); *(BYTE*)((fs->win+484)+1)=(BYTE)((WORD)(0x61417272)>>8); *(BYTE*)((fs->win+484)+2)=(BYTE)((DWORD)(0x61417272)>>16); *(BYTE*)((fs->win+484)+3)=(BYTE)((DWORD)(0x61417272)>>24);
			*(BYTE*)(fs->win+488)=(BYTE)(fs->free_clust); *(BYTE*)((fs->win+488)+1)=(BYTE)((WORD)(fs->free_clust)>>8); *(BYTE*)((fs->win+488)+2)=(BYTE)((DWORD)(fs->free_clust)>>16); *(BYTE*)((fs->win+488)+3)=(BYTE)((DWORD)(fs->free_clust)>>24);
			*(BYTE*)(fs->win+492)=(BYTE)(fs->last_clust); *(BYTE*)((fs->win+492)+1)=(BYTE)((WORD)(fs->last_clust)>>8); *(BYTE*)((fs->win+492)+2)=(BYTE)((DWORD)(fs->last_clust)>>16); *(BYTE*)((fs->win+492)+3)=(BYTE)((DWORD)(fs->last_clust)>>24);
			disk_write(fs->drv, fs->win, fs->fsi_sector, 1);
			fs->fsi_flag = 0;
		}
		 
		if (disk_ioctl(fs->drv, 0, (void*)0) != RES_OK)
			res = FR_DISK_ERR;
	}

	return res;
}





 
 
 


DWORD get_fat (	 
	FATFS *fs,	 
	DWORD clst	 
)
{
	UINT wc, bc;
	BYTE *p;


	if (clst < 2 || clst >= fs->n_fatent)	 
		return 1;

	switch (fs->fs_type) {
	case 1 :
		bc = (UINT)clst; bc += bc / 2;
		if (move_window(fs, fs->fatbase + (bc / 512U))) break;
		wc = fs->win[bc % 512U]; bc++;
		if (move_window(fs, fs->fatbase + (bc / 512U))) break;
		wc |= fs->win[bc % 512U] << 8;
		return (clst & 1) ? (wc >> 4) : (wc & 0xFFF);

	case 2 :
		if (move_window(fs, fs->fatbase + (clst / (512U / 2)))) break;
		p = &fs->win[clst * 2 % 512U];
		return (WORD)(((WORD)*(BYTE*)((p)+1)<<8)|(WORD)*(BYTE*)(p));

	case 3 :
		if (move_window(fs, fs->fatbase + (clst / (512U / 4)))) break;
		p = &fs->win[clst * 4 % 512U];
		return (DWORD)(((DWORD)*(BYTE*)((p)+3)<<24)|((DWORD)*(BYTE*)((p)+2)<<16)|((WORD)*(BYTE*)((p)+1)<<8)| *(BYTE*)(p)) & 0x0FFFFFFF;
	}

	return 0xFFFFFFFF;	 
}




 
 
 


FRESULT put_fat (
	FATFS *fs,	 
	DWORD clst,	 
	DWORD val	 
)
{
	UINT bc;
	BYTE *p;
	FRESULT res;


	if (clst < 2 || clst >= fs->n_fatent) {	 
		res = FR_INT_ERR;

	} else {
		switch (fs->fs_type) {
		case 1 :
			bc = clst; bc += bc / 2;
			res = move_window(fs, fs->fatbase + (bc / 512U));
			if (res != FR_OK) break;
			p = &fs->win[bc % 512U];
			*p = (clst & 1) ? ((*p & 0x0F) | ((BYTE)val << 4)) : (BYTE)val;
			bc++;
			fs->wflag = 1;
			res = move_window(fs, fs->fatbase + (bc / 512U));
			if (res != FR_OK) break;
			p = &fs->win[bc % 512U];
			*p = (clst & 1) ? (BYTE)(val >> 4) : ((*p & 0xF0) | ((BYTE)(val >> 8) & 0x0F));
			break;

		case 2 :
			res = move_window(fs, fs->fatbase + (clst / (512U / 2)));
			if (res != FR_OK) break;
			p = &fs->win[clst * 2 % 512U];
			*(BYTE*)(p)=(BYTE)((WORD)val); *(BYTE*)((p)+1)=(BYTE)((WORD)((WORD)val)>>8);
			break;

		case 3 :
			res = move_window(fs, fs->fatbase + (clst / (512U / 4)));
			if (res != FR_OK) break;
			p = &fs->win[clst * 4 % 512U];
			val |= (DWORD)(((DWORD)*(BYTE*)((p)+3)<<24)|((DWORD)*(BYTE*)((p)+2)<<16)|((WORD)*(BYTE*)((p)+1)<<8)| *(BYTE*)(p)) & 0xF0000000;
			*(BYTE*)(p)=(BYTE)(val); *(BYTE*)((p)+1)=(BYTE)((WORD)(val)>>8); *(BYTE*)((p)+2)=(BYTE)((DWORD)(val)>>16); *(BYTE*)((p)+3)=(BYTE)((DWORD)(val)>>24);
			break;

		default :
			res = FR_INT_ERR;
		}
		fs->wflag = 1;
	}

	return res;
}





 
 
 

static
FRESULT remove_chain (
	FATFS *fs,			 
	DWORD clst			 
)
{
	FRESULT res;
	DWORD nxt;


	if (clst < 2 || clst >= fs->n_fatent) {	 
		res = FR_INT_ERR;

	} else {
		res = FR_OK;
		while (clst < fs->n_fatent) {			 
			nxt = get_fat(fs, clst);			 
			if (nxt == 0) break;				 
			if (nxt == 1) { res = FR_INT_ERR; break; }	 
			if (nxt == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }	 
			res = put_fat(fs, clst, 0);			 
			if (res != FR_OK) break;
			if (fs->free_clust != 0xFFFFFFFF) {	 
				fs->free_clust++;
				fs->fsi_flag = 1;
			}
			clst = nxt;	 
		}
	}

	return res;
}





 
 
 

static
DWORD create_chain (	 
	FATFS *fs,			 
	DWORD clst			 
)
{
	DWORD cs, ncl, scl;


	if (clst == 0) {		 
		scl = fs->last_clust;			 
		if (!scl || scl >= fs->n_fatent) scl = 1;
	}
	else {					 
		cs = get_fat(fs, clst);			 
		if (cs < 2) return 1;			 
		if (cs < fs->n_fatent) return cs;	 
		scl = clst;
	}

	ncl = scl;				 
	for (;;) {
		ncl++;							 
		if (ncl >= fs->n_fatent) {		 
			ncl = 2;
			if (ncl > scl) return 0;	 
		}
		cs = get_fat(fs, ncl);			 
		if (cs == 0) break;				 
		if (cs == 0xFFFFFFFF || cs == 1) 
			return cs;
		if (ncl == scl) return 0;		 
	}

	if (put_fat(fs, ncl, 0x0FFFFFFF))	 
		return 0xFFFFFFFF;
	if (clst != 0) {					 
		if (put_fat(fs, clst, ncl))
			return 0xFFFFFFFF;
	}

	fs->last_clust = ncl;				 
	if (fs->free_clust != 0xFFFFFFFF) {
		fs->free_clust--;
		fs->fsi_flag = 1;
	}

	return ncl;		 
}





 
 
 


DWORD clust2sect (	 
	FATFS *fs,		 
	DWORD clst		 
)
{
	clst -= 2;
	if (clst >= (fs->n_fatent - 2)) return 0;		 
	return clst * fs->csize + fs->database;
}




 
 
 

static
FRESULT dir_sdi (
	DIR *dj,		 
	WORD idx		 
)
{
	DWORD clst;
	WORD ic;


	dj->index = idx;
	clst = dj->sclust;
	if (clst == 1 || clst >= dj->fs->n_fatent)	 
		return FR_INT_ERR;
	if (!clst && dj->fs->fs_type == 3)	 
		clst = dj->fs->dirbase;

	if (clst == 0) {	 
		dj->clust = clst;
		if (idx >= dj->fs->n_rootdir)		 
			return FR_INT_ERR;
		dj->sect = dj->fs->dirbase + idx / (512U / 32);	 
	}
	else {				 
		ic = 512U / 32 * dj->fs->csize;	 
		while (idx >= ic) {	 
			clst = get_fat(dj->fs, clst);				 
			if (clst == 0xFFFFFFFF) return FR_DISK_ERR;	 
			if (clst < 2 || clst >= dj->fs->n_fatent)	 
				return FR_INT_ERR;
			idx -= ic;
		}
		dj->clust = clst;
		dj->sect = clust2sect(dj->fs, clst) + idx / (512U / 32);	 
	}

	dj->dir = dj->fs->win + (idx % (512U / 32)) * 32;	 

	return FR_OK;	 
}




 
 
 

static
FRESULT dir_next (	 
	DIR *dj,		 
	int stretch		 
)
{
	DWORD clst;
	WORD i;


	i = dj->index + 1;
	if (!i || !dj->sect)	 
		return FR_NO_FILE;

	if (!(i % (512U / 32))) {	 
		dj->sect++;					 

		if (dj->clust == 0) {	 
			if (i >= dj->fs->n_rootdir)	 
				return FR_NO_FILE;
		}
		else {					 
			if (((i / (512U / 32)) & (dj->fs->csize - 1)) == 0) {	 
				clst = get_fat(dj->fs, dj->clust);				 
				if (clst <= 1) return FR_INT_ERR;
				if (clst == 0xFFFFFFFF) return FR_DISK_ERR;
				if (clst >= dj->fs->n_fatent) {					 

					BYTE c;
					if (!stretch) return FR_NO_FILE;			 
					clst = create_chain(dj->fs, dj->clust);		 
					if (clst == 0) return FR_DENIED;			 
					if (clst == 1) return FR_INT_ERR;
					if (clst == 0xFFFFFFFF) return FR_DISK_ERR;
					 
					if (move_window(dj->fs, 0)) return FR_DISK_ERR;	 
					mem_set(dj->fs->win, 0, 512U);			 
					dj->fs->winsect = clust2sect(dj->fs, clst);	 
					for (c = 0; c < dj->fs->csize; c++) {		 
						dj->fs->wflag = 1;
						if (move_window(dj->fs, 0)) return FR_DISK_ERR;
						dj->fs->winsect++;
					}
					dj->fs->winsect -= c;						 



				}
				dj->clust = clst;				 
				dj->sect = clust2sect(dj->fs, clst);
			}
		}
	}

	dj->index = i;
	dj->dir = dj->fs->win + (i % (512U / 32)) * 32;

	return FR_OK;
}




 
 
 

static
const BYTE LfnOfs[] = {1,3,5,7,9,14,16,18,20,22,24,28,30};	 


static
int cmp_lfn (			 
	WCHAR *lfnbuf,		 
	BYTE *dir			 
)
{
	int i, s;
	WCHAR wc, uc;


	i = ((dir[0] & 0xBF) - 1) * 13;	 
	s = 0; wc = 1;
	do {
		uc = (WORD)(((WORD)*(BYTE*)((dir+LfnOfs[s])+1)<<8)|(WORD)*(BYTE*)(dir+LfnOfs[s]));	 
		if (wc) {	 
			wc = ff_wtoupper(uc);		 
			if (i >= 127 || wc != ff_wtoupper(lfnbuf[i++]))	 
				return 0;				 
		} else {
			if (uc != 0xFFFF) return 0;	 
		}
	} while (++s < 13);				 

	if ((dir[0] & 0x40) && wc && lfnbuf[i])	 
		return 0;

	return 1;						 
}



static
int pick_lfn (			 
	WCHAR *lfnbuf,		 
	BYTE *dir			 
)
{
	int i, s;
	WCHAR wc, uc;


	i = ((dir[0] & 0x3F) - 1) * 13;	 

	s = 0; wc = 1;
	do {
		uc = (WORD)(((WORD)*(BYTE*)((dir+LfnOfs[s])+1)<<8)|(WORD)*(BYTE*)(dir+LfnOfs[s]));		 
		if (wc) {	 
			if (i >= 127) return 0;	 
			lfnbuf[i++] = wc = uc;			 
		} else {
			if (uc != 0xFFFF) return 0;		 
		}
	} while (++s < 13);						 

	if (dir[0] & 0x40) {				 
		if (i >= 127) return 0;		 
		lfnbuf[i] = 0;
	}

	return 1;
}



static
void fit_lfn (
	const WCHAR *lfnbuf,	 
	BYTE *dir,				 
	BYTE ord,				 
	BYTE sum				 
)
{
	int i, s;
	WCHAR wc;


	dir[13] = sum;			 
	dir[11] = 0x0F;		 
	dir[12] = 0;
	*(BYTE*)(dir+26)=(BYTE)(0); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(0)>>8);

	i = (ord - 1) * 13;				 
	s = wc = 0;
	do {
		if (wc != 0xFFFF) wc = lfnbuf[i++];	 
		*(BYTE*)(dir+LfnOfs[s])=(BYTE)(wc); *(BYTE*)((dir+LfnOfs[s])+1)=(BYTE)((WORD)(wc)>>8);	 
		if (!wc) wc = 0xFFFF;		 
	} while (++s < 13);
	if (wc == 0xFFFF || !lfnbuf[i]) ord |= 0x40;	 
	dir[0] = ord;			 
}






 
 
 

void gen_numname (
	BYTE *dst,			 
	const BYTE *src,	 
	const WCHAR *lfn,	 
	WORD seq			 
)
{
	BYTE ns[8], c;
	int i, j;


	mem_cpy(dst, src, 11);

	if (seq > 5) {	 
		do seq = (seq >> 1) + (seq << 15) + (WORD)*lfn++; while (*lfn);
	}

	 
	i = 7;
	do {
		c = (seq % 16) + '0';
		if (c > '9') c += 7;
		ns[i--] = c;
		seq /= 16;
	} while (seq);
	ns[i] = '~';

	 
	for (j = 0; j < i && dst[j] != ' '; j++) {
		if (0) {
			if (j == i - 1) break;
			j++;
		}
	}
	do {
		dst[j++] = (i < 8) ? ns[i++] : ' ';
	} while (j < 8);
}





 
 
 

static
BYTE sum_sfn (
	const BYTE *dir		 
)
{
	BYTE sum = 0;
	int n = 11;

	do sum = (sum >> 1) + (sum << 7) + *dir++; while (--n);
	return sum;
}





 
 
 

static
FRESULT dir_find (
	DIR *dj			 
)
{
	FRESULT res;
	BYTE c, *dir;

	BYTE a, ord, sum;


	res = dir_sdi(dj, 0);			 
	if (res != FR_OK) return res;


	ord = sum = 0xFF;

	do {
		res = move_window(dj->fs, dj->sect);
		if (res != FR_OK) break;
		dir = dj->dir;					 
		c = dir[0];
		if (c == 0) { res = FR_NO_FILE; break; }	 

		a = dir[11] & 0x3F;
		if (c == 0xE5 || ((a & 0x08) && a != 0x0F)) {	 
			ord = 0xFF;
		} else {
			if (a == 0x0F) {			 
				if (dj->lfn) {
					if (c & 0x40) {		 
						sum = dir[13];
						c &= 0xBF; ord = c;	 
						dj->lfn_idx = dj->index;
					}
					 
					ord = (c == ord && sum == dir[13] && cmp_lfn(dj->lfn, dir)) ? ord - 1 : 0xFF;
				}
			} else {					 
				if (!ord && sum == sum_sfn(dir)) break;	 
				ord = 0xFF; dj->lfn_idx = 0xFFFF;	 
				if (!(dj->fn[11] & 0x01) && !mem_cmp(dir, dj->fn, 11)) break;	 
			}
		}




		res = dir_next(dj, 0);		 
	} while (res == FR_OK);

	return res;
}




 
 
 

static
FRESULT dir_read (
	DIR *dj			 
)
{
	FRESULT res;
	BYTE c, *dir;

	BYTE a, ord = 0xFF, sum = 0xFF;


	res = FR_NO_FILE;
	while (dj->sect) {
		res = move_window(dj->fs, dj->sect);
		if (res != FR_OK) break;
		dir = dj->dir;					 
		c = dir[0];
		if (c == 0) { res = FR_NO_FILE; break; }	 

		a = dir[11] & 0x3F;
		if (c == 0xE5 || (!1 && c == '.') || ((a & 0x08) && a != 0x0F)) {	 
			ord = 0xFF;
		} else {
			if (a == 0x0F) {			 
				if (c & 0x40) {			 
					sum = dir[13];
					c &= 0xBF; ord = c;
					dj->lfn_idx = dj->index;
				}
				 
				ord = (c == ord && sum == dir[13] && pick_lfn(dj->lfn, dir)) ? ord - 1 : 0xFF;
			} else {					 
				if (ord || sum != sum_sfn(dir))	 
					dj->lfn_idx = 0xFFFF;		 
				break;
			}
		}




		res = dir_next(dj, 0);				 
		if (res != FR_OK) break;
	}

	if (res != FR_OK) dj->sect = 0;

	return res;
}




 
 
 

static
FRESULT dir_register (	 
	DIR *dj				 
)
{
	FRESULT res;
	BYTE c, *dir;

	WORD n, ne, is;
	BYTE sn[12], *fn, sum;
	WCHAR *lfn;


	fn = dj->fn; lfn = dj->lfn;
	mem_cpy(sn, fn, 12);

	if (1 && (sn[11] & 0x20)) return FR_INVALID_NAME;	 

	if (sn[11] & 0x01) {			 
		fn[11] = 0; dj->lfn = 0;			 
		for (n = 1; n < 100; n++) {
			gen_numname(fn, sn, lfn, n);	 
			res = dir_find(dj);				 
			if (res != FR_OK) break;
		}
		if (n == 100) return FR_DENIED;		 
		if (res != FR_NO_FILE) return res;	 
		fn[11] = sn[11]; dj->lfn = lfn;
	}

	if (sn[11] & 0x02) {			 
		for (ne = 0; lfn[ne]; ne++) ;
		ne = (ne + 25) / 13;
	} else {						 
		ne = 1;
	}

	 
	res = dir_sdi(dj, 0);
	if (res != FR_OK) return res;
	n = is = 0;
	do {
		res = move_window(dj->fs, dj->sect);
		if (res != FR_OK) break;
		c = *dj->dir;				 
		if (c == 0xE5 || c == 0) {	 
			if (n == 0) is = dj->index;	 
			if (++n == ne) break;	 
		} else {
			n = 0;					 
		}
		res = dir_next(dj, 1);		 
	} while (res == FR_OK);

	if (res == FR_OK && ne > 1) {	 
		res = dir_sdi(dj, is);
		if (res == FR_OK) {
			sum = sum_sfn(dj->fn);	 
			ne--;
			do {					 
				res = move_window(dj->fs, dj->sect);
				if (res != FR_OK) break;
				fit_lfn(dj->lfn, dj->dir, (BYTE)ne, sum);
				dj->fs->wflag = 1;
				res = dir_next(dj, 0);	 
			} while (res == FR_OK && --ne);
		}
	}

#line 1197 "ff8\\src\\ff.c"

	if (res == FR_OK) {		 
		res = move_window(dj->fs, dj->sect);
		if (res == FR_OK) {
			dir = dj->dir;
			mem_set(dir, 0, 32);		 
			mem_cpy(dir, dj->fn, 11);	 

			dir[12] = *(dj->fn+11) & (0x08 | 0x10);	 

			dj->fs->wflag = 1;
		}
	}

	return res;
}





 
 
 

static
FRESULT dir_remove (	 
	DIR *dj				 
)
{
	FRESULT res;

	WORD i;

	i = dj->index;	 
	res = dir_sdi(dj, (WORD)((dj->lfn_idx == 0xFFFF) ? i : dj->lfn_idx));	 
	if (res == FR_OK) {
		do {
			res = move_window(dj->fs, dj->sect);
			if (res != FR_OK) break;
			*dj->dir = 0xE5;			 
			dj->fs->wflag = 1;
			if (dj->index >= i) break;	 
			res = dir_next(dj, 0);		 
		} while (res == FR_OK);
		if (res == FR_NO_FILE) res = FR_INT_ERR;
	}

#line 1255 "ff8\\src\\ff.c"

	return res;
}





 
 
 

static
FRESULT create_name (
	DIR *dj,			 
	const TCHAR **path	 
)
{

	static const BYTE excvt[] = {0x80,0x9A,0x90,0xB6,0x8E,0xB7,0x8F,0x80,0xD2,0xD3,0xD4,0xD8,0xD7,0xDE,0x8E,0x8F,0x90,0x92,0x92,0xE2,0x99,0xE3,0xEA,0xEB,0x59,0x99,0x9A,0x9D,0x9C,0x9D,0x9E,0x9F, 0xB5,0xD6,0xE0,0xE9,0xA5,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0x21,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF, 0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC7,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF, 0xE0,0xE1,0xE2,0xE3,0xE5,0xE5,0xE6,0xE7,0xE7,0xE9,0xEA,0xEB,0xED,0xED,0xEE,0xEF,0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF};	 



	BYTE b, cf;
	WCHAR w, *lfn;
	int i, ni, si, di;
	const TCHAR *p;

	 
	si = di = 0;
	p = *path;
	lfn = dj->lfn;
	for (;;) {
		w = p[si++];					 
		if (w < ' ' || w == '/' || w == '\\') break;	 
		if (di >= 127)				 
			return FR_INVALID_NAME;

		w &= 0xFF;
		if (0) {				 
			b = p[si++];				 
			if (!0)			 
				return FR_INVALID_NAME;



		}
		w = ff_convert(w, 1);			 
		if (!w) return FR_INVALID_NAME;	 

		if (w < 0x80 && chk_chr("\"*:<>\?|\x7F", w))  
			return FR_INVALID_NAME;
		lfn[di++] = w;					 
	}
	*path = &p[si];						 
	cf = (w < ' ') ? 0x04 : 0;		 

	if ((di == 1 && lfn[di - 1] == '.') ||  
		(di == 2 && lfn[di - 1] == '.' && lfn[di - 2] == '.')) {
		lfn[di] = 0;
		for (i = 0; i < 11; i++)
			dj->fn[i] = (i < di) ? '.' : ' ';
		dj->fn[i] = cf | 0x20;		 
		return FR_OK;
	}

	while (di) {						 
		w = lfn[di - 1];
		if (w != ' ' && w != '.') break;
		di--;
	}
	if (!di) return FR_INVALID_NAME;	 

	lfn[di] = 0;						 

	 
	mem_set(dj->fn, ' ', 11);
	for (si = 0; lfn[si] == ' ' || lfn[si] == '.'; si++) ;	 
	if (si) cf |= 0x01 | 0x02;
	while (di && lfn[di - 1] != '.') di--;	 

	b = i = 0; ni = 8;
	for (;;) {
		w = lfn[si++];					 
		if (!w) break;					 
		if (w == ' ' || (w == '.' && si != di)) {	 
			cf |= 0x01 | 0x02; continue;
		}

		if (i >= ni || si == di) {		 
			if (ni == 11) {				 
				cf |= 0x01 | 0x02; break;
			}
			if (si != di) cf |= 0x01 | 0x02;	 
			if (si > di) break;			 
			si = di; i = 8; ni = 11;	 
			b <<= 2; continue;
		}

		if (w >= 0x80) {				 

			w = ff_convert(w, 0);		 
			if (w) w = excvt[w - 0x80];	 



			cf |= 0x02;				 
		}

		if (0 && w >= 0x100) {		 
			if (i >= ni - 1) {
				cf |= 0x01 | 0x02; i = ni; continue;
			}
			dj->fn[i++] = (BYTE)(w >> 8);
		} else {						 
			if (!w || chk_chr("+,;=[]", w)) {		 
				w = '_'; cf |= 0x01 | 0x02;	 
			} else {
				if ((((w)>= 'A')&&((w)<= 'Z'))) {		 
					b |= 2;
				} else {
					if ((((w)>= 'a')&&((w)<= 'z'))) {	 
						b |= 1; w -= 0x20;
					}
				}
			}
		}
		dj->fn[i++] = (BYTE)w;
	}

	if (dj->fn[0] == 0xE5) dj->fn[0] = 0x05;	 

	if (ni == 8) b <<= 2;
	if ((b & 0x0C) == 0x0C || (b & 0x03) == 0x03)	 
		cf |= 0x02;
	if (!(cf & 0x02)) {						 
		if ((b & 0x03) == 0x01) cf |= 0x10;	 
		if ((b & 0x0C) == 0x04) cf |= 0x08;	 
	}

	dj->fn[11] = cf;	 

	return FR_OK;


#line 1474 "ff8\\src\\ff.c"
}




 
 
 

static
void get_fileinfo (		 
	DIR *dj,			 
	FILINFO *fno	 	 
)
{
	int i;
	BYTE nt, *dir;
	TCHAR *p, c;


	p = fno->fname;
	if (dj->sect) {
		dir = dj->dir;
		nt = dir[12];		 
		for (i = 0; i < 8; i++) {	 
			c = dir[i];
			if (c == ' ') break;
			if (c == 0x05) c = (TCHAR)0xE5;
			if (1 && (nt & 0x08) && (((c)>= 'A')&&((c)<= 'Z'))) c += 0x20;
#line 1509 "ff8\\src\\ff.c"
			*p++ = c;
		}
		if (dir[8] != ' ') {		 
			*p++ = '.';
			for (i = 8; i < 11; i++) {
				c = dir[i];
				if (c == ' ') break;
				if (1 && (nt & 0x10) && (((c)>= 'A')&&((c)<= 'Z'))) c += 0x20;
#line 1523 "ff8\\src\\ff.c"
				*p++ = c;
			}
		}
		fno->fattrib = dir[11];				 
		fno->fsize = (DWORD)(((DWORD)*(BYTE*)((dir+28)+3)<<24)|((DWORD)*(BYTE*)((dir+28)+2)<<16)|((WORD)*(BYTE*)((dir+28)+1)<<8)| *(BYTE*)(dir+28));	 
		fno->fdate = (WORD)(((WORD)*(BYTE*)((dir+24)+1)<<8)|(WORD)*(BYTE*)(dir+24));		 
		fno->ftime = (WORD)(((WORD)*(BYTE*)((dir+22)+1)<<8)|(WORD)*(BYTE*)(dir+22));		 
	}
	*p = 0;


	if (fno->lfname) {
		TCHAR *tp = fno->lfname;
		WCHAR w, *lfn;

		i = 0;
		if (dj->sect && dj->lfn_idx != 0xFFFF) { 
			lfn = dj->lfn;
			while ((w = *lfn++) != 0) {			 

				w = ff_convert(w, 0);			 
				if (!w) { i = 0; break; }		 
				if (0 && w >= 0x100)		 
					tp[i++] = (TCHAR)(w >> 8);

				if (i >= fno->lfsize - 1) { i = 0; break; }	 
				tp[i++] = (TCHAR)w;
			}
		}
		tp[i] = 0;	 
	}

}





 
 
 

static
FRESULT follow_path (	 
	DIR *dj,			 
	const TCHAR *path	 
)
{
	FRESULT res;
	BYTE *dir, ns;



	if (*path == '/' || *path == '\\') {  
		path++;	dj->sclust = 0;		 
	} else {							 
		dj->sclust = dj->fs->cdir;	 
	}






	if ((UINT)*path < ' ') {			 
		res = dir_sdi(dj, 0);
		dj->dir = 0;

	} else {							 
		for (;;) {
			res = create_name(dj, &path);	 
			if (res != FR_OK) break;
			res = dir_find(dj);				 
			ns = *(dj->fn+11);
			if (res != FR_OK) {				 
				if (res != FR_NO_FILE) break;	 
				 
				if (1 && (ns & 0x20)) {	 
					dj->sclust = 0; dj->dir = 0;	 
					res = FR_OK;
					if (!(ns & 0x04)) continue;
				} else {							 
					if (!(ns & 0x04)) res = FR_NO_PATH;
				}
				break;
			}
			if (ns & 0x04) break;			 
			dir = dj->dir;						 
			if (!(dir[11] & 0x10)) {	 
				res = FR_NO_PATH; break;
			}
			dj->sclust = ((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));
		}
	}

	return res;
}




 
 
 

static
BYTE check_fs (	 
	FATFS *fs,	 
	DWORD sect	 
)
{
	if (disk_read(fs->drv, fs->win, sect, 1) != RES_OK)	 
		return 3;
	if ((WORD)(((WORD)*(BYTE*)((&fs->win[510])+1)<<8)|(WORD)*(BYTE*)(&fs->win[510])) != 0xAA55)		 
		return 2;

	if (((DWORD)(((DWORD)*(BYTE*)((&fs->win[54])+3)<<24)|((DWORD)*(BYTE*)((&fs->win[54])+2)<<16)|((WORD)*(BYTE*)((&fs->win[54])+1)<<8)| *(BYTE*)(&fs->win[54])) & 0xFFFFFF) == 0x544146)	 
		return 0;
	if (((DWORD)(((DWORD)*(BYTE*)((&fs->win[82])+3)<<24)|((DWORD)*(BYTE*)((&fs->win[82])+2)<<16)|((WORD)*(BYTE*)((&fs->win[82])+1)<<8)| *(BYTE*)(&fs->win[82])) & 0xFFFFFF) == 0x544146)
		return 0;

	return 1;
}




 
 
 

static
FRESULT chk_mounted (	 
	const TCHAR **path,	 
	FATFS **rfs,		 
	BYTE chk_wp			 
)
{
	BYTE fmt, b, *tbl;
	UINT vol;
	DSTATUS stat;
	DWORD bsect, fasize, tsect, sysect, nclst, szbfat;
	WORD nrsv;
	const TCHAR *p = *path;
	FATFS *fs;

	 
	vol = p[0] - '0';				 
	if (vol <= 9 && p[1] == ':') {	 
		p += 2; *path = p;			 
	} else {						 

		vol = Drive;				 



	}

	 
	if (vol >= 1) 			 
		return FR_INVALID_DRIVE;
	*rfs = fs = FatFs[vol];			 
	if (!fs) return FR_NOT_ENABLED;	 

	;					 

	if (fs->fs_type) {				 
		stat = disk_status(fs->drv);
		if (!(stat & 0x01)) {	 

			if (chk_wp && (stat & 0x04))	 
				return FR_WRITE_PROTECTED;

			return FR_OK;			 
		}
	}

	 

	fs->fs_type = 0;					 
	fs->drv = (BYTE)(vol);			 
	stat = disk_initialize(fs->drv);	 
	if (stat & 0x01)				 
		return FR_NOT_READY;





	if (chk_wp && (stat & 0x04))	 
		return FR_WRITE_PROTECTED;

	 
	fmt = check_fs(fs, bsect = 0);		 
	if (fmt == 1) {						 
		 
		tbl = &fs->win[446 + 0 * 16];	 
		if (tbl[4]) {									 
			bsect = (DWORD)(((DWORD)*(BYTE*)((&tbl[8])+3)<<24)|((DWORD)*(BYTE*)((&tbl[8])+2)<<16)|((WORD)*(BYTE*)((&tbl[8])+1)<<8)| *(BYTE*)(&tbl[8]));					 
			fmt = check_fs(fs, bsect);					 
		}
	}
	if (fmt == 3) return FR_DISK_ERR;
	if (fmt) return FR_NO_FILESYSTEM;					 

	 

	if ((WORD)(((WORD)*(BYTE*)((fs->win+11)+1)<<8)|(WORD)*(BYTE*)(fs->win+11)) != 512U)		 
		return FR_NO_FILESYSTEM;

	fasize = (WORD)(((WORD)*(BYTE*)((fs->win+22)+1)<<8)|(WORD)*(BYTE*)(fs->win+22));				 
	if (!fasize) fasize = (DWORD)(((DWORD)*(BYTE*)((fs->win+36)+3)<<24)|((DWORD)*(BYTE*)((fs->win+36)+2)<<16)|((WORD)*(BYTE*)((fs->win+36)+1)<<8)| *(BYTE*)(fs->win+36));
	fs->fsize = fasize;

	fs->n_fats = b = fs->win[16];				 
	if (b != 1 && b != 2) return FR_NO_FILESYSTEM;		 
	fasize *= b;										 

	fs->csize = b = fs->win[13];			 
	if (!b || (b & (b - 1))) return FR_NO_FILESYSTEM;	 

	fs->n_rootdir = (WORD)(((WORD)*(BYTE*)((fs->win+17)+1)<<8)|(WORD)*(BYTE*)(fs->win+17));	 
	if (fs->n_rootdir % (512U / 32)) return FR_NO_FILESYSTEM;	 

	tsect = (WORD)(((WORD)*(BYTE*)((fs->win+19)+1)<<8)|(WORD)*(BYTE*)(fs->win+19));				 
	if (!tsect) tsect = (DWORD)(((DWORD)*(BYTE*)((fs->win+32)+3)<<24)|((DWORD)*(BYTE*)((fs->win+32)+2)<<16)|((WORD)*(BYTE*)((fs->win+32)+1)<<8)| *(BYTE*)(fs->win+32));

	nrsv = (WORD)(((WORD)*(BYTE*)((fs->win+14)+1)<<8)|(WORD)*(BYTE*)(fs->win+14));				 
	if (!nrsv) return FR_NO_FILESYSTEM;					 

	 
	sysect = nrsv + fasize + fs->n_rootdir / (512U / 32);	 
	if (tsect < sysect) return FR_NO_FILESYSTEM;		 
	nclst = (tsect - sysect) / fs->csize;				 
	if (!nclst) return FR_NO_FILESYSTEM;				 
	fmt = 1;
	if (nclst >= 4086) fmt = 2;
	if (nclst >= 65526) fmt = 3;

	 
	fs->n_fatent = nclst + 2;							 
	fs->database = bsect + sysect;						 
	fs->fatbase = bsect + nrsv; 						 
	if (fmt == 3) {
		if (fs->n_rootdir) return FR_NO_FILESYSTEM;		 
		fs->dirbase = (DWORD)(((DWORD)*(BYTE*)((fs->win+44)+3)<<24)|((DWORD)*(BYTE*)((fs->win+44)+2)<<16)|((WORD)*(BYTE*)((fs->win+44)+1)<<8)| *(BYTE*)(fs->win+44));	 
		szbfat = fs->n_fatent * 4;						 
	} else {
		if (!fs->n_rootdir)	return FR_NO_FILESYSTEM;	 
		fs->dirbase = fs->fatbase + fasize;				 
		szbfat = (fmt == 2) ?					 
			fs->n_fatent * 2 : fs->n_fatent * 3 / 2 + (fs->n_fatent & 1);
	}
	if (fs->fsize < (szbfat + (512U - 1)) / 512U)	 
		return FR_NO_FILESYSTEM;


	 
	fs->free_clust = 0xFFFFFFFF;
	fs->last_clust = 0;

	 
	if (fmt == 3) {
	 	fs->fsi_flag = 0;
		fs->fsi_sector = bsect + (WORD)(((WORD)*(BYTE*)((fs->win+48)+1)<<8)|(WORD)*(BYTE*)(fs->win+48));
		if (disk_read(fs->drv, fs->win, fs->fsi_sector, 1) == RES_OK &&
			(WORD)(((WORD)*(BYTE*)((fs->win+510)+1)<<8)|(WORD)*(BYTE*)(fs->win+510)) == 0xAA55 &&
			(DWORD)(((DWORD)*(BYTE*)((fs->win+0)+3)<<24)|((DWORD)*(BYTE*)((fs->win+0)+2)<<16)|((WORD)*(BYTE*)((fs->win+0)+1)<<8)| *(BYTE*)(fs->win+0)) == 0x41615252 &&
			(DWORD)(((DWORD)*(BYTE*)((fs->win+484)+3)<<24)|((DWORD)*(BYTE*)((fs->win+484)+2)<<16)|((WORD)*(BYTE*)((fs->win+484)+1)<<8)| *(BYTE*)(fs->win+484)) == 0x61417272) {
				fs->last_clust = (DWORD)(((DWORD)*(BYTE*)((fs->win+492)+3)<<24)|((DWORD)*(BYTE*)((fs->win+492)+2)<<16)|((WORD)*(BYTE*)((fs->win+492)+1)<<8)| *(BYTE*)(fs->win+492));
				fs->free_clust = (DWORD)(((DWORD)*(BYTE*)((fs->win+488)+3)<<24)|((DWORD)*(BYTE*)((fs->win+488)+2)<<16)|((WORD)*(BYTE*)((fs->win+488)+1)<<8)| *(BYTE*)(fs->win+488));
		}
	}

	fs->fs_type = fmt;		 
	fs->id = ++Fsid;		 
	fs->winsect = 0;		 
	fs->wflag = 0;

	fs->cdir = 0;			 






	return FR_OK;
}




 
 
 

static
FRESULT validate (	 
	FATFS *fs,		 
	WORD id			 
)
{
	if (!fs || !fs->fs_type || fs->id != id)
		return FR_INVALID_OBJECT;

	;		 

	if (disk_status(fs->drv) & 0x01)
		return FR_NOT_READY;

	return FR_OK;
}








 



 
 
 

FRESULT f_mount (
	BYTE vol,		 
	FATFS *fs		 
)
{
	FATFS *rfs;


	if (vol >= 1)				 
		return FR_INVALID_DRIVE;
	rfs = FatFs[vol];				 

	if (rfs) {



		rfs->fs_type = 0;			 
	}

	if (fs) {
		fs->fs_type = 0;			 



	}
	FatFs[vol] = fs;				 

	return FR_OK;
}




 
 
 

FRESULT f_open (
	FIL *fp,			 
	const TCHAR *path,	 
	BYTE mode			 
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	BYTE sfn[12];


	fp->fs = 0;			 


	mode &= 0x01 | 0x02 | 0x08 | 0x10 | 0x04;
	res = chk_mounted(&path, &dj.fs, (BYTE)(mode & ~0x01));




	{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
	if (res == FR_OK)
		res = follow_path(&dj, path);	 
	dir = dj.dir;


	if (res == FR_OK) {
		if (!dir)	 
			res = FR_INVALID_NAME;




	}
	 
	if (mode & (0x08 | 0x10 | 0x04)) {
		DWORD dw, cl;

		if (res != FR_OK) {				 
			if (res == FR_NO_FILE)		 



				res = dir_register(&dj);

			mode |= 0x08;
			dir = dj.dir;				 
		}
		else {							 
			if (mode & 0x04) {			 
				res = FR_EXIST;
			} else {
				if (dir[11] & (0x01 | 0x10))	 
					res = FR_DENIED;
			}
		}
		if (res == FR_OK && (mode & 0x08)) {	 
			dw = get_fattime();						 
			*(BYTE*)(dir+14)=(BYTE)(dw); *(BYTE*)((dir+14)+1)=(BYTE)((WORD)(dw)>>8); *(BYTE*)((dir+14)+2)=(BYTE)((DWORD)(dw)>>16); *(BYTE*)((dir+14)+3)=(BYTE)((DWORD)(dw)>>24);
			dir[11] = 0;					 
			*(BYTE*)(dir+28)=(BYTE)(0); *(BYTE*)((dir+28)+1)=(BYTE)((WORD)(0)>>8); *(BYTE*)((dir+28)+2)=(BYTE)((DWORD)(0)>>16); *(BYTE*)((dir+28)+3)=(BYTE)((DWORD)(0)>>24);		 
			cl = ((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));	 
			*(BYTE*)(dir+20)=(BYTE)(0); *(BYTE*)((dir+20)+1)=(BYTE)((WORD)(0)>>8);		 
			*(BYTE*)(dir+26)=(BYTE)(0); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(0)>>8);
			dj.fs->wflag = 1;
			if (cl) {							 
				dw = dj.fs->winsect;
				res = remove_chain(dj.fs, cl);
				if (res == FR_OK) {
					dj.fs->last_clust = cl - 1;	 
					res = move_window(dj.fs, dw);
				}
			}
		}
	}
	else {	 
		if (res == FR_OK) {						 
			if (dir[11] & 0x10) {		 
				res = FR_NO_FILE;
			} else {
				if ((mode & 0x02) && (dir[11] & 0x01))  
					res = FR_DENIED;
			}
		}
	}
	if (res == FR_OK) {
		if (mode & (0x02 | 0x08 | 0x10 | 0x04))
			mode |= 0x20;				 
		fp->dir_sect = dj.fs->winsect;			 
		fp->dir_ptr = dir;




	}

#line 1995 "ff8\\src\\ff.c"
	;

	if (res == FR_OK) {
		fp->flag = mode;					 
		fp->org_clust =						 
			((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));
		fp->fsize = (DWORD)(((DWORD)*(BYTE*)((dir+28)+3)<<24)|((DWORD)*(BYTE*)((dir+28)+2)<<16)|((WORD)*(BYTE*)((dir+28)+1)<<8)| *(BYTE*)(dir+28));	 
		fp->fptr = 0;						 
		fp->dsect = 0;



		fp->fs = dj.fs; fp->id = dj.fs->id;	 
	}

	return res;
}




 
 
 

FRESULT f_read (
	FIL *fp, 		 
	void *buff,		 
	UINT btr,		 
	UINT *br		 
)
{
	FRESULT res;
	DWORD clst, sect, remain;
	UINT rcnt, cc;
	BYTE csect, *rbuff = buff;


	*br = 0;	 

	res = validate(fp->fs, fp->id);					 
	if (res != FR_OK) return res;
	if (fp->flag & 0x80)						 
		return FR_INT_ERR;
	if (!(fp->flag & 0x01)) 						 
		return FR_DENIED;
	remain = fp->fsize - fp->fptr;
	if (btr > remain) btr = (UINT)remain;			 

	for ( ;  btr;									 
		rbuff += rcnt, fp->fptr += rcnt, *br += rcnt, btr -= rcnt) {
		if ((fp->fptr % 512U) == 0) {			 
			csect = (BYTE)(fp->fptr / 512U & (fp->fs->csize - 1));	 
			if (!csect) {							 
				clst = (fp->fptr == 0) ?			 
					fp->org_clust : get_fat(fp->fs, fp->curr_clust);
				if (clst <= 1) { fp->flag |= 0x80; return FR_INT_ERR; };
				if (clst == 0xFFFFFFFF) { fp->flag |= 0x80; return FR_DISK_ERR; };
				fp->curr_clust = clst;				 
			}
			sect = clust2sect(fp->fs, fp->curr_clust);	 
			if (!sect) { fp->flag |= 0x80; return FR_INT_ERR; };
			sect += csect;
			cc = btr / 512U;					 
			if (cc) {								 
				if (csect + cc > fp->fs->csize)		 
					cc = fp->fs->csize - csect;
				if (disk_read(fp->fs->drv, rbuff, sect, (BYTE)cc) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };





				if ((fp->flag & 0x40) && fp->dsect - sect < cc)
					mem_cpy(rbuff + ((fp->dsect - sect) * 512U), fp->buf, 512U);


				rcnt = 512U * cc;				 
				continue;
			}


			if (fp->flag & 0x40) {			 
				if (disk_write(fp->fs->drv, fp->buf, fp->dsect, 1) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };
				fp->flag &= ~0x40;
			}

			if (fp->dsect != sect) {			 
				if (disk_read(fp->fs->drv, fp->buf, sect, 1) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };
			}

			fp->dsect = sect;
		}
		rcnt = 512U - (fp->fptr % 512U);	 
		if (rcnt > btr) rcnt = btr;





		mem_cpy(rbuff, &fp->buf[fp->fptr % 512U], rcnt);	 

	}

	return FR_OK;
}





 
 
 

FRESULT f_write (
	FIL *fp,			 
	const void *buff,	 
	UINT btw,			 
	UINT *bw			 
)
{
	FRESULT res;
	DWORD clst, sect;
	UINT wcnt, cc;
	const BYTE *wbuff = buff;
	BYTE csect;


	*bw = 0;	 

	res = validate(fp->fs, fp->id);					 
	if (res != FR_OK) return res;
	if (fp->flag & 0x80)						 
		return FR_INT_ERR;
	if (!(fp->flag & 0x02))						 
		return FR_DENIED;
	if (fp->fsize + btw < fp->fsize) btw = 0;		 

	for ( ;  btw;									 
		wbuff += wcnt, fp->fptr += wcnt, *bw += wcnt, btw -= wcnt) {
		if ((fp->fptr % 512U) == 0) {			 
			csect = (BYTE)(fp->fptr / 512U & (fp->fs->csize - 1));	 
			if (!csect) {							 
				if (fp->fptr == 0) {				 
					clst = fp->org_clust;			 
					if (clst == 0)					 
						fp->org_clust = clst = create_chain(fp->fs, 0);	 
				} else {							 
					clst = create_chain(fp->fs, fp->curr_clust);			 
				}
				if (clst == 0) break;				 
				if (clst == 1) { fp->flag |= 0x80; return FR_INT_ERR; };
				if (clst == 0xFFFFFFFF) { fp->flag |= 0x80; return FR_DISK_ERR; };
				fp->curr_clust = clst;				 
			}




			if (fp->flag & 0x40) {		 
				if (disk_write(fp->fs->drv, fp->buf, fp->dsect, 1) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };
				fp->flag &= ~0x40;
			}

			sect = clust2sect(fp->fs, fp->curr_clust);	 
			if (!sect) { fp->flag |= 0x80; return FR_INT_ERR; };
			sect += csect;
			cc = btw / 512U;					 
			if (cc) {								 
				if (csect + cc > fp->fs->csize)		 
					cc = fp->fs->csize - csect;
				if (disk_write(fp->fs->drv, wbuff, sect, (BYTE)cc) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };
#line 2179 "ff8\\src\\ff.c"
				if (fp->dsect - sect < cc) {		 
					mem_cpy(fp->buf, wbuff + ((fp->dsect - sect) * 512U), 512U);
					fp->flag &= ~0x40;
				}

				wcnt = 512U * cc;				 
				continue;
			}
#line 2193 "ff8\\src\\ff.c"
			if (fp->dsect != sect) {				 
				if (fp->fptr < fp->fsize &&
					disk_read(fp->fs->drv, fp->buf, sect, 1) != RES_OK)
						{ fp->flag |= 0x80; return FR_DISK_ERR; };
			}

			fp->dsect = sect;
		}
		wcnt = 512U - (fp->fptr % 512U);	 
		if (wcnt > btw) wcnt = btw;
#line 2209 "ff8\\src\\ff.c"
		mem_cpy(&fp->buf[fp->fptr % 512U], wbuff, wcnt);	 
		fp->flag |= 0x40;

	}

	if (fp->fptr > fp->fsize) fp->fsize = fp->fptr;	 
	fp->flag |= 0x20;						 

	return FR_OK;
}




 
 
 

FRESULT f_sync (
	FIL *fp		 
)
{
	FRESULT res;
	DWORD tim;
	BYTE *dir;


	res = validate(fp->fs, fp->id);		 
	if (res == FR_OK) {
		if (fp->flag & 0x20) {	 

			if (fp->flag & 0x40) {
				if (disk_write(fp->fs->drv, fp->buf, fp->dsect, 1) != RES_OK)
					return FR_DISK_ERR;
				fp->flag &= ~0x40;
			}

			 
			res = move_window(fp->fs, fp->dir_sect);
			if (res == FR_OK) {
				dir = fp->dir_ptr;
				dir[11] |= 0x20;					 
				*(BYTE*)(dir+28)=(BYTE)(fp->fsize); *(BYTE*)((dir+28)+1)=(BYTE)((WORD)(fp->fsize)>>8); *(BYTE*)((dir+28)+2)=(BYTE)((DWORD)(fp->fsize)>>16); *(BYTE*)((dir+28)+3)=(BYTE)((DWORD)(fp->fsize)>>24);		 
				*(BYTE*)(dir+26)=(BYTE)(fp->org_clust); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(fp->org_clust)>>8);	 
				*(BYTE*)(dir+20)=(BYTE)(fp->org_clust >> 16); *(BYTE*)((dir+20)+1)=(BYTE)((WORD)(fp->org_clust >> 16)>>8);
				tim = get_fattime();						 
				*(BYTE*)(dir+22)=(BYTE)(tim); *(BYTE*)((dir+22)+1)=(BYTE)((WORD)(tim)>>8); *(BYTE*)((dir+22)+2)=(BYTE)((DWORD)(tim)>>16); *(BYTE*)((dir+22)+3)=(BYTE)((DWORD)(tim)>>24);
				fp->flag &= ~0x20;
				fp->fs->wflag = 1;
				res = sync(fp->fs);
			}
		}
	}

	return res;
}






 
 
 

FRESULT f_close (
	FIL *fp		 
)
{
	FRESULT res;

#line 2288 "ff8\\src\\ff.c"
	res = f_sync(fp);		 
#line 2302 "ff8\\src\\ff.c"
	if (res == FR_OK) fp->fs = 0;	 
	return res;

}




 
 
 



FRESULT f_chdrive (
	BYTE drv		 
)
{
	if (drv >= 1) return FR_INVALID_DRIVE;

	Drive = drv;

	return FR_OK;
}




FRESULT f_chdir (
	const TCHAR *path	 
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 0);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);		 
		;
		if (res == FR_OK) {					 
			dir = dj.dir;					 
			if (!dir) {
				dj.fs->cdir = dj.sclust;	 
			} else {
				if (dir[11] & 0x10)	 
					dj.fs->cdir = ((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));
				else
					res = FR_NO_PATH;		 
			}
		}
		if (res == FR_NO_FILE) res = FR_NO_PATH;
	}

	return res;
}






 
 
 

FRESULT f_lseek (
	FIL *fp,		 
	DWORD ofs		 
)
{
	FRESULT res;


	res = validate(fp->fs, fp->id);		 
	if (res != FR_OK) return res;
	if (fp->flag & 0x80)			 
		return FR_INT_ERR;

#line 2446 "ff8\\src\\ff.c"

	 
	{
		DWORD clst, bcs, nsect, ifptr;

		if (ofs > fp->fsize					 

			 && !(fp->flag & 0x02)

			) ofs = fp->fsize;

		ifptr = fp->fptr;
		fp->fptr = nsect = 0;
		if (ofs) {
			bcs = (DWORD)fp->fs->csize * 512U;	 
			if (ifptr > 0 &&
				(ofs - 1) / bcs >= (ifptr - 1) / bcs) {	 
				fp->fptr = (ifptr - 1) & ~(bcs - 1);	 
				ofs -= fp->fptr;
				clst = fp->curr_clust;
			} else {									 
				clst = fp->org_clust;					 

				if (clst == 0) {						 
					clst = create_chain(fp->fs, 0);
					if (clst == 1) { fp->flag |= 0x80; return FR_INT_ERR; };
					if (clst == 0xFFFFFFFF) { fp->flag |= 0x80; return FR_DISK_ERR; };
					fp->org_clust = clst;
				}

				fp->curr_clust = clst;
			}
			if (clst != 0) {
				while (ofs > bcs) {						 

					if (fp->flag & 0x02) {			 
						clst = create_chain(fp->fs, clst);	 
						if (clst == 0) {				 
							ofs = bcs; break;
						}
					} else

						clst = get_fat(fp->fs, clst);	 
					if (clst == 0xFFFFFFFF) { fp->flag |= 0x80; return FR_DISK_ERR; };
					if (clst <= 1 || clst >= fp->fs->n_fatent) { fp->flag |= 0x80; return FR_INT_ERR; };
					fp->curr_clust = clst;
					fp->fptr += bcs;
					ofs -= bcs;
				}
				fp->fptr += ofs;
				if (ofs % 512U) {
					nsect = clust2sect(fp->fs, clst);	 
					if (!nsect) { fp->flag |= 0x80; return FR_INT_ERR; };
					nsect += ofs / 512U;
				}
			}
		}
		if (fp->fptr % 512U && nsect != fp->dsect) {


			if (fp->flag & 0x40) {			 
				if (disk_write(fp->fs->drv, fp->buf, fp->dsect, 1) != RES_OK)
					{ fp->flag |= 0x80; return FR_DISK_ERR; };
				fp->flag &= ~0x40;
			}

			if (disk_read(fp->fs->drv, fp->buf, nsect, 1) != RES_OK)
				{ fp->flag |= 0x80; return FR_DISK_ERR; };

			fp->dsect = nsect;
		}

		if (fp->fptr > fp->fsize) {			 
			fp->fsize = fp->fptr;
			fp->flag |= 0x20;
		}

	}

	return res;
}




 
 
 

FRESULT f_opendir (
	DIR *dj,			 
	const TCHAR *path	 
)
{
	FRESULT res;
	BYTE *dir;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj->fs, 0);
	if (res == FR_OK) {
		{ (*dj). fn = sfn; (*dj). lfn = LfnBuf; };
		res = follow_path(dj, path);			 
		;
		if (res == FR_OK) {						 
			dir = dj->dir;
			if (dir) {							 
				if (dir[11] & 0x10) {	 
					dj->sclust = ((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));
				} else {						 
					res = FR_NO_PATH;
				}
			}
			if (res == FR_OK) {
				dj->id = dj->fs->id;
				res = dir_sdi(dj, 0);			 
			}
		}
		if (res == FR_NO_FILE) res = FR_NO_PATH;
	}

	return res;
}




 
 
 

FRESULT f_readdir (
	DIR *dj,			 
	FILINFO *fno		 
)
{
	FRESULT res;
	BYTE sfn[12];


	res = validate(dj->fs, dj->id);			 
	if (res == FR_OK) {
		if (!fno) {
			res = dir_sdi(dj, 0);
		} else {
			{ (*dj). fn = sfn; (*dj). lfn = LfnBuf; };
			res = dir_read(dj);
			if (res == FR_NO_FILE) {
				dj->sect = 0;
				res = FR_OK;
			}
			if (res == FR_OK) {				 
				get_fileinfo(dj, fno);		 
				res = dir_next(dj, 0);		 
				if (res == FR_NO_FILE) {
					dj->sect = 0;
					res = FR_OK;
				}
			}
			;
		}
	}

	return res;
}




 
 
 

FRESULT f_stat (
	const TCHAR *path,	 
	FILINFO *fno		 
)
{
	FRESULT res;
	DIR dj;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 0);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);	 
		if (res == FR_OK) {				 
			if (dj.dir)		 
				get_fileinfo(&dj, fno);
			else			 
				res = FR_INVALID_NAME;
		}
		;
	}

	return res;
}




 
 
 

FRESULT f_getfree (
	const TCHAR *path,	 
	DWORD *nclst,		 
	FATFS **fatfs		 
)
{
	FRESULT res;
	DWORD n, clst, sect, stat;
	UINT i;
	BYTE fat, *p;


	 
	res = chk_mounted(&path, fatfs, 0);
	if (res == FR_OK) {
		 
		if ((*fatfs)->free_clust <= (*fatfs)->n_fatent - 2) {
			*nclst = (*fatfs)->free_clust;
		} else {
			 
			fat = (*fatfs)->fs_type;
			n = 0;
			if (fat == 1) {
				clst = 2;
				do {
					stat = get_fat(*fatfs, clst);
					if (stat == 0xFFFFFFFF) { res = FR_DISK_ERR; break; }
					if (stat == 1) { res = FR_INT_ERR; break; }
					if (stat == 0) n++;
				} while (++clst < (*fatfs)->n_fatent);
			} else {
				clst = (*fatfs)->n_fatent;
				sect = (*fatfs)->fatbase;
				i = 0; p = 0;
				do {
					if (!i) {
						res = move_window(*fatfs, sect++);
						if (res != FR_OK) break;
						p = (*fatfs)->win;
						i = 512U;
					}
					if (fat == 2) {
						if ((WORD)(((WORD)*(BYTE*)((p)+1)<<8)|(WORD)*(BYTE*)(p)) == 0) n++;
						p += 2; i -= 2;
					} else {
						if (((DWORD)(((DWORD)*(BYTE*)((p)+3)<<24)|((DWORD)*(BYTE*)((p)+2)<<16)|((WORD)*(BYTE*)((p)+1)<<8)| *(BYTE*)(p)) & 0x0FFFFFFF) == 0) n++;
						p += 4; i -= 4;
					}
				} while (--clst);
			}
			(*fatfs)->free_clust = n;
			if (fat == 3) (*fatfs)->fsi_flag = 1;
			*nclst = n;
		}
	}
	return res;
}




 
 
 

FRESULT f_truncate (
	FIL *fp		 
)
{
	FRESULT res;
	DWORD ncl;


	res = validate(fp->fs, fp->id);		 
	if (res == FR_OK) {
		if (fp->flag & 0x80) {			 
			res = FR_INT_ERR;
		} else {
			if (!(fp->flag & 0x02))		 
				res = FR_DENIED;
		}
	}
	if (res == FR_OK) {
		if (fp->fsize > fp->fptr) {
			fp->fsize = fp->fptr;	 
			fp->flag |= 0x20;
			if (fp->fptr == 0) {	 
				res = remove_chain(fp->fs, fp->org_clust);
				fp->org_clust = 0;
			} else {				 
				ncl = get_fat(fp->fs, fp->curr_clust);
				res = FR_OK;
				if (ncl == 0xFFFFFFFF) res = FR_DISK_ERR;
				if (ncl == 1) res = FR_INT_ERR;
				if (res == FR_OK && ncl < fp->fs->n_fatent) {
					res = put_fat(fp->fs, fp->curr_clust, 0x0FFFFFFF);
					if (res == FR_OK) res = remove_chain(fp->fs, ncl);
				}
			}
		}
		if (res != FR_OK) fp->flag |= 0x80;
	}

	return res;
}




 
 
 

FRESULT f_unlink (
	const TCHAR *path		 
)
{
	FRESULT res;
	DIR dj, sdj;
	BYTE *dir;
	DWORD dclst;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 1);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);		 
		if (1 && res == FR_OK && (dj.fn[11] & 0x20))
			res = FR_INVALID_NAME;			 



		if (res == FR_OK) {					 
			dir = dj.dir;
			if (!dir) {
				res = FR_INVALID_NAME;		 
			} else {
				if (dir[11] & 0x01)
					res = FR_DENIED;		 
			}
			dclst = ((DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) << 16) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26));
			if (res == FR_OK && (dir[11] & 0x10)) {	 
				if (dclst < 2) {
					res = FR_INT_ERR;
				} else {
					mem_cpy(&sdj, &dj, sizeof(DIR));	 
					sdj.sclust = dclst;
					res = dir_sdi(&sdj, 2);		 
					if (res == FR_OK) {
						res = dir_read(&sdj);
						if (res == FR_OK			 

						|| dclst == sdj.fs->cdir	 

						) res = FR_DENIED;
						if (res == FR_NO_FILE) res = FR_OK;	 
					}
				}
			}
			if (res == FR_OK) {
				res = dir_remove(&dj);		 
				if (res == FR_OK) {
					if (dclst)				 
						res = remove_chain(dj.fs, dclst);
					if (res == FR_OK) res = sync(dj.fs);
				}
			}
		}
		;
	}
	return res;
}




 
 
 

FRESULT f_mkdir (
	const TCHAR *path		 
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir, n;
	DWORD dsc, dcl, pcl, tim = get_fattime();
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 1);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);			 
		if (res == FR_OK) res = FR_EXIST;		 
		if (1 && res == FR_NO_FILE && (dj.fn[11] & 0x20))
			res = FR_INVALID_NAME;
		if (res == FR_NO_FILE) {				 
			dcl = create_chain(dj.fs, 0);		 
			res = FR_OK;
			if (dcl == 0) res = FR_DENIED;		 
			if (dcl == 1) res = FR_INT_ERR;
			if (dcl == 0xFFFFFFFF) res = FR_DISK_ERR;
			if (res == FR_OK)					 
				res = move_window(dj.fs, 0);
			if (res == FR_OK) {					 
				dsc = clust2sect(dj.fs, dcl);
				dir = dj.fs->win;
				mem_set(dir, 0, 512U);
				mem_set(dir+0, ' ', 8+3);	 
				dir[0] = '.';
				dir[11] = 0x10;
				*(BYTE*)(dir+22)=(BYTE)(tim); *(BYTE*)((dir+22)+1)=(BYTE)((WORD)(tim)>>8); *(BYTE*)((dir+22)+2)=(BYTE)((DWORD)(tim)>>16); *(BYTE*)((dir+22)+3)=(BYTE)((DWORD)(tim)>>24);
				*(BYTE*)(dir+26)=(BYTE)(dcl); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(dcl)>>8);
				*(BYTE*)(dir+20)=(BYTE)(dcl >> 16); *(BYTE*)((dir+20)+1)=(BYTE)((WORD)(dcl >> 16)>>8);
				mem_cpy(dir+32, dir, 32); 			 
				dir[33] = '.'; pcl = dj.sclust;
				if (dj.fs->fs_type == 3 && pcl == dj.fs->dirbase)
					pcl = 0;
				*(BYTE*)(dir+32+26)=(BYTE)(pcl); *(BYTE*)((dir+32+26)+1)=(BYTE)((WORD)(pcl)>>8);
				*(BYTE*)(dir+32+20)=(BYTE)(pcl >> 16); *(BYTE*)((dir+32+20)+1)=(BYTE)((WORD)(pcl >> 16)>>8);
				for (n = dj.fs->csize; n; n--) {	 
					dj.fs->winsect = dsc++;
					dj.fs->wflag = 1;
					res = move_window(dj.fs, 0);
					if (res != FR_OK) break;
					mem_set(dir, 0, 512U);
				}
			}
			if (res == FR_OK) res = dir_register(&dj);	 
			if (res != FR_OK) {
				remove_chain(dj.fs, dcl);				 
			} else {
				dir = dj.dir;
				dir[11] = 0x10;					 
				*(BYTE*)(dir+22)=(BYTE)(tim); *(BYTE*)((dir+22)+1)=(BYTE)((WORD)(tim)>>8); *(BYTE*)((dir+22)+2)=(BYTE)((DWORD)(tim)>>16); *(BYTE*)((dir+22)+3)=(BYTE)((DWORD)(tim)>>24);			 
				*(BYTE*)(dir+26)=(BYTE)(dcl); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(dcl)>>8);		 
				*(BYTE*)(dir+20)=(BYTE)(dcl >> 16); *(BYTE*)((dir+20)+1)=(BYTE)((WORD)(dcl >> 16)>>8);
				dj.fs->wflag = 1;
				res = sync(dj.fs);
			}
		}
		;
	}

	return res;
}




 
 
 

FRESULT f_chmod (
	const TCHAR *path,	 
	BYTE value,			 
	BYTE mask			 
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 1);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);		 
		;
		if (1 && res == FR_OK && (dj.fn[11] & 0x20))
			res = FR_INVALID_NAME;
		if (res == FR_OK) {
			dir = dj.dir;
			if (!dir) {						 
				res = FR_INVALID_NAME;
			} else {						 
				mask &= 0x01|0x02|0x04|0x20;	 
				dir[11] = (value & mask) | (dir[11] & (BYTE)~mask);	 
				dj.fs->wflag = 1;
				res = sync(dj.fs);
			}
		}
	}

	return res;
}




 
 
 

FRESULT f_utime (
	const TCHAR *path,	 
	const FILINFO *fno	 
)
{
	FRESULT res;
	DIR dj;
	BYTE *dir;
	BYTE sfn[12];


	res = chk_mounted(&path, &dj.fs, 1);
	if (res == FR_OK) {
		{ (dj). fn = sfn; (dj). lfn = LfnBuf; };
		res = follow_path(&dj, path);	 
		;
		if (1 && res == FR_OK && (dj.fn[11] & 0x20))
			res = FR_INVALID_NAME;
		if (res == FR_OK) {
			dir = dj.dir;
			if (!dir) {					 
				res = FR_INVALID_NAME;
			} else {					 
				*(BYTE*)(dir+22)=(BYTE)(fno->ftime); *(BYTE*)((dir+22)+1)=(BYTE)((WORD)(fno->ftime)>>8);
				*(BYTE*)(dir+24)=(BYTE)(fno->fdate); *(BYTE*)((dir+24)+1)=(BYTE)((WORD)(fno->fdate)>>8);
				dj.fs->wflag = 1;
				res = sync(dj.fs);
			}
		}
	}

	return res;
}




 
 
 

FRESULT f_rename (
	const TCHAR *path_old,	 
	const TCHAR *path_new	 
)
{
	FRESULT res;
	DIR djo, djn;
	BYTE buf[21], *dir;
	DWORD dw;
	BYTE sfn[12];


	res = chk_mounted(&path_old, &djo.fs, 1);
	if (res == FR_OK) {
		djn.fs = djo.fs;
		{ (djo). fn = sfn; (djo). lfn = LfnBuf; };
		res = follow_path(&djo, path_old);		 
		if (1 && res == FR_OK && (djo.fn[11] & 0x20))
			res = FR_INVALID_NAME;



		if (res == FR_OK) {						 
			if (!djo.dir) {						 
				res = FR_NO_FILE;
			} else {
				mem_cpy(buf, djo.dir+11, 21);		 
				mem_cpy(&djn, &djo, sizeof(DIR));		 
				res = follow_path(&djn, path_new);
				if (res == FR_OK) res = FR_EXIST;			 
				if (res == FR_NO_FILE) { 					 
 
					res = dir_register(&djn);			 
					if (res == FR_OK) {
						dir = djn.dir;					 
						mem_cpy(dir+13, buf+2, 19);
						dir[11] = buf[0] | 0x20;
						djo.fs->wflag = 1;
						if (djo.sclust != djn.sclust && (dir[11] & 0x10)) {		 
							dw = clust2sect(djn.fs, (DWORD)(WORD)(((WORD)*(BYTE*)((dir+20)+1)<<8)|(WORD)*(BYTE*)(dir+20)) | (WORD)(((WORD)*(BYTE*)((dir+26)+1)<<8)|(WORD)*(BYTE*)(dir+26)));
							if (!dw) {
								res = FR_INT_ERR;
							} else {
								res = move_window(djn.fs, dw);
								dir = djn.fs->win+32;	 
								if (res == FR_OK && dir[1] == '.') {
									dw = (djn.fs->fs_type == 3 && djn.sclust == djn.fs->dirbase) ? 0 : djn.sclust;
									*(BYTE*)(dir+26)=(BYTE)(dw); *(BYTE*)((dir+26)+1)=(BYTE)((WORD)(dw)>>8);
									*(BYTE*)(dir+20)=(BYTE)(dw >> 16); *(BYTE*)((dir+20)+1)=(BYTE)((WORD)(dw >> 16)>>8);
									djn.fs->wflag = 1;
								}
							}
						}
						if (res == FR_OK) {
							res = dir_remove(&djo);		 
							if (res == FR_OK)
								res = sync(djo.fs);
						}
					}
 
				}
			}
		}
		;
	}
	return res;
}








 
 
 
#line 3124 "ff8\\src\\ff.c"




 
 
 




FRESULT f_mkfs (
	BYTE drv,		 
	BYTE sfd,		 
	UINT au			 
)
{
	static const WORD vst[] = { 1024,   512,  256,  128,   64,    32,   16,    8,    4,    2,   0};
	static const WORD cst[] = {32768, 16384, 8192, 4096, 2048, 16384, 8192, 4096, 2048, 1024, 512};
	BYTE fmt, md, *tbl;
	DWORD n_clst, vs, n;
	UINT as, i;
	DWORD b_vol, b_fat, b_dir, b_data;		 
	DWORD n_vol, n_rsv, n_fat, n_dir;		 
	FATFS *fs;
	DSTATUS stat;


	 
	if (drv >= 1) return FR_INVALID_DRIVE;
	fs = FatFs[drv];
	if (!fs) return FR_NOT_ENABLED;
	fs->fs_type = 0;
	drv = (drv);

	 
	stat = disk_initialize(drv);
	if (stat & 0x01) return FR_NOT_READY;
	if (stat & 0x04) return FR_WRITE_PROTECTED;




	if (disk_ioctl(drv, 1, &n_vol) != RES_OK || n_vol < 128)
		return FR_DISK_ERR;
	b_vol = (sfd == 1) ? 0 : 63;	 
	n_vol -= b_vol;
	if (au & (au - 1)) au = 0;		 
	if (!au) {						 
		vs = n_vol / (2000 / (512U / 512));
		for (i = 0; vs < vst[i]; i++) ;
		au = cst[i];
	}
	if (512 != 512 && au < 512U) au = 512U;
	au /= 512U;		 
	if (au == 0) au = 1;
	if (au > 128) au = 128;

	 
	n_clst = n_vol / au;
	fmt = 1;
	if (n_clst >= 4086) fmt = 2;
	if (n_clst >= 65526) fmt = 3;

	 
	if (fmt == 3) {
		n_fat = ((n_clst * 4) + 8 + 512U - 1) / 512U;
		n_rsv = 32;
		n_dir = 0;
	} else {
		n_fat = (fmt == 1) ? (n_clst * 3 + 1) / 2 + 3 : (n_clst * 2) + 4;
		n_fat = (n_fat + 512U - 1) / 512U;
		n_rsv = 1;
		n_dir = 512 * 32UL / 512U;
	}
	b_fat = b_vol + n_rsv;				 
	b_dir = b_fat + n_fat * 1;		 
	b_data = b_dir + n_dir;				 
	if (n_vol < b_data + au) return FR_MKFS_ABORTED;	 

	 
	if (disk_ioctl(drv, 3, &n) != RES_OK) return FR_DISK_ERR;
	if (!n || n > 32768) return FR_MKFS_ABORTED;
	n = (b_data + n - 1) & ~(n - 1);	 
	n = (n - b_data) / 1;
	if (fmt == 3) {		 
		n_rsv += n;
		b_fat += n;
	} else {					 
		n_fat += n;
	}
	 

	 
	n_clst = (n_vol - n_rsv - n_fat * 1 - n_dir) / au;
	if (   (fmt == 2 && n_clst < 4086)
		|| (fmt == 3 && n_clst < 65526))
		return FR_MKFS_ABORTED;

	 
	if (sfd == 1) {
		md = 0xF0;
	} else {
		DWORD n_disk = b_vol + n_vol;

		mem_set(fs->win, 0, 512U);
		tbl = fs->win+446;
		*(BYTE*)(tbl)=(BYTE)(0x00010180); *(BYTE*)((tbl)+1)=(BYTE)((WORD)(0x00010180)>>8); *(BYTE*)((tbl)+2)=(BYTE)((DWORD)(0x00010180)>>16); *(BYTE*)((tbl)+3)=(BYTE)((DWORD)(0x00010180)>>24);		 
		if (n_disk < 63UL * 255 * 1024) {	 
			n_disk = n_disk / 63 / 255;
			tbl[7] = (BYTE)n_disk;
			tbl[6] = (BYTE)((n_disk >> 2) | 63);
		} else {
			*(BYTE*)(&tbl[6])=(BYTE)(0xFFFF); *(BYTE*)((&tbl[6])+1)=(BYTE)((WORD)(0xFFFF)>>8);
		}
		tbl[5] = 254;
		if (fmt != 3)			 
			tbl[4] = (n_vol < 0x10000) ? 0x04 : 0x06;
		else
			tbl[4] = 0x0c;
		*(BYTE*)(tbl+8)=(BYTE)(63); *(BYTE*)((tbl+8)+1)=(BYTE)((WORD)(63)>>8); *(BYTE*)((tbl+8)+2)=(BYTE)((DWORD)(63)>>16); *(BYTE*)((tbl+8)+3)=(BYTE)((DWORD)(63)>>24);			 
		*(BYTE*)(tbl+12)=(BYTE)(n_vol); *(BYTE*)((tbl+12)+1)=(BYTE)((WORD)(n_vol)>>8); *(BYTE*)((tbl+12)+2)=(BYTE)((DWORD)(n_vol)>>16); *(BYTE*)((tbl+12)+3)=(BYTE)((DWORD)(n_vol)>>24);		 
		*(BYTE*)(tbl+64)=(BYTE)(0xAA55); *(BYTE*)((tbl+64)+1)=(BYTE)((WORD)(0xAA55)>>8);		 
		if (disk_write(drv, fs->win, 0, 1) != RES_OK)
			return FR_DISK_ERR;
		md = 0xF8;
	}

	 
	tbl = fs->win;								 
	mem_set(tbl, 0, 512U);
	*(BYTE*)(tbl+0)=(BYTE)(0x90FEEB); *(BYTE*)((tbl+0)+1)=(BYTE)((WORD)(0x90FEEB)>>8); *(BYTE*)((tbl+0)+2)=(BYTE)((DWORD)(0x90FEEB)>>16); *(BYTE*)((tbl+0)+3)=(BYTE)((DWORD)(0x90FEEB)>>24);			 
	as = 512U;								 
	*(BYTE*)(tbl+11)=(BYTE)(as); *(BYTE*)((tbl+11)+1)=(BYTE)((WORD)(as)>>8);
	tbl[13] = (BYTE)au;				 
	*(BYTE*)(tbl+14)=(BYTE)(n_rsv); *(BYTE*)((tbl+14)+1)=(BYTE)((WORD)(n_rsv)>>8);			 
	tbl[16] = 1;					 
	as = (fmt == 3) ? 0 : 512;		 
	*(BYTE*)(tbl+17)=(BYTE)(as); *(BYTE*)((tbl+17)+1)=(BYTE)((WORD)(as)>>8);
	if (n_vol < 0x10000) {						 
		*(BYTE*)(tbl+19)=(BYTE)(n_vol); *(BYTE*)((tbl+19)+1)=(BYTE)((WORD)(n_vol)>>8);
	} else {
		*(BYTE*)(tbl+32)=(BYTE)(n_vol); *(BYTE*)((tbl+32)+1)=(BYTE)((WORD)(n_vol)>>8); *(BYTE*)((tbl+32)+2)=(BYTE)((DWORD)(n_vol)>>16); *(BYTE*)((tbl+32)+3)=(BYTE)((DWORD)(n_vol)>>24);
	}
	tbl[21] = md;						 
	*(BYTE*)(tbl+24)=(BYTE)(63); *(BYTE*)((tbl+24)+1)=(BYTE)((WORD)(63)>>8);				 
	*(BYTE*)(tbl+26)=(BYTE)(255); *(BYTE*)((tbl+26)+1)=(BYTE)((WORD)(255)>>8);				 
	*(BYTE*)(tbl+28)=(BYTE)(b_vol); *(BYTE*)((tbl+28)+1)=(BYTE)((WORD)(b_vol)>>8); *(BYTE*)((tbl+28)+2)=(BYTE)((DWORD)(b_vol)>>16); *(BYTE*)((tbl+28)+3)=(BYTE)((DWORD)(b_vol)>>24);			 
	n = get_fattime();							 
	if (fmt == 3) {
		*(BYTE*)(tbl+67)=(BYTE)(n); *(BYTE*)((tbl+67)+1)=(BYTE)((WORD)(n)>>8); *(BYTE*)((tbl+67)+2)=(BYTE)((DWORD)(n)>>16); *(BYTE*)((tbl+67)+3)=(BYTE)((DWORD)(n)>>24);			 
		*(BYTE*)(tbl+36)=(BYTE)(n_fat); *(BYTE*)((tbl+36)+1)=(BYTE)((WORD)(n_fat)>>8); *(BYTE*)((tbl+36)+2)=(BYTE)((DWORD)(n_fat)>>16); *(BYTE*)((tbl+36)+3)=(BYTE)((DWORD)(n_fat)>>24);		 
		*(BYTE*)(tbl+44)=(BYTE)(2); *(BYTE*)((tbl+44)+1)=(BYTE)((WORD)(2)>>8); *(BYTE*)((tbl+44)+2)=(BYTE)((DWORD)(2)>>16); *(BYTE*)((tbl+44)+3)=(BYTE)((DWORD)(2)>>24);			 
		*(BYTE*)(tbl+48)=(BYTE)(1); *(BYTE*)((tbl+48)+1)=(BYTE)((WORD)(1)>>8);				 
		*(BYTE*)(tbl+50)=(BYTE)(6); *(BYTE*)((tbl+50)+1)=(BYTE)((WORD)(6)>>8);			 
		tbl[64] = 0x80;				 
		tbl[66] = 0x29;				 
		mem_cpy(tbl+71, "NO NAME    FAT32   ", 19);	 
	} else {
		*(BYTE*)(tbl+39)=(BYTE)(n); *(BYTE*)((tbl+39)+1)=(BYTE)((WORD)(n)>>8); *(BYTE*)((tbl+39)+2)=(BYTE)((DWORD)(n)>>16); *(BYTE*)((tbl+39)+3)=(BYTE)((DWORD)(n)>>24);				 
		*(BYTE*)(tbl+22)=(BYTE)(n_fat); *(BYTE*)((tbl+22)+1)=(BYTE)((WORD)(n_fat)>>8);		 
		tbl[36] = 0x80;					 
		tbl[38] = 0x29;					 
		mem_cpy(tbl+43, "NO NAME    FAT     ", 19);	 
	}
	*(BYTE*)(tbl+510)=(BYTE)(0xAA55); *(BYTE*)((tbl+510)+1)=(BYTE)((WORD)(0xAA55)>>8);				 
	if (disk_write(drv, tbl, b_vol, 1) != RES_OK)	 
		return FR_DISK_ERR;
	if (fmt == 3)						 
		disk_write(drv, tbl, b_vol + 6, 1);

	 
	for (i = 0; i < 1; i++) {
		mem_set(tbl, 0, 512U);			 
		n = md;								 
		if (fmt != 3) {
			n |= (fmt == 1) ? 0x00FFFF00 : 0xFFFFFF00;
			*(BYTE*)(tbl+0)=(BYTE)(n); *(BYTE*)((tbl+0)+1)=(BYTE)((WORD)(n)>>8); *(BYTE*)((tbl+0)+2)=(BYTE)((DWORD)(n)>>16); *(BYTE*)((tbl+0)+3)=(BYTE)((DWORD)(n)>>24);				 
		} else {
			n |= 0x0FFFFF00;
			*(BYTE*)(tbl+0)=(BYTE)(n); *(BYTE*)((tbl+0)+1)=(BYTE)((WORD)(n)>>8); *(BYTE*)((tbl+0)+2)=(BYTE)((DWORD)(n)>>16); *(BYTE*)((tbl+0)+3)=(BYTE)((DWORD)(n)>>24);				 
			*(BYTE*)(tbl+4)=(BYTE)(0x0FFFFFFF); *(BYTE*)((tbl+4)+1)=(BYTE)((WORD)(0x0FFFFFFF)>>8); *(BYTE*)((tbl+4)+2)=(BYTE)((DWORD)(0x0FFFFFFF)>>16); *(BYTE*)((tbl+4)+3)=(BYTE)((DWORD)(0x0FFFFFFF)>>24);
			*(BYTE*)(tbl+8)=(BYTE)(0x0FFFFFFF); *(BYTE*)((tbl+8)+1)=(BYTE)((WORD)(0x0FFFFFFF)>>8); *(BYTE*)((tbl+8)+2)=(BYTE)((DWORD)(0x0FFFFFFF)>>16); *(BYTE*)((tbl+8)+3)=(BYTE)((DWORD)(0x0FFFFFFF)>>24);	 
		}
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_DISK_ERR;
		mem_set(tbl, 0, 512U);		 
		for (n = 1; n < n_fat; n++) {	 
			if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
				return FR_DISK_ERR;
		}
	}

	 
	n = (fmt == 3) ? as : n_dir;
	while (n--) {
		if (disk_write(drv, tbl, b_fat++, 1) != RES_OK)
			return FR_DISK_ERR;
	}

	 
	if (fmt == 3) {
		*(BYTE*)(tbl+510)=(BYTE)(0xAA55); *(BYTE*)((tbl+510)+1)=(BYTE)((WORD)(0xAA55)>>8);
		*(BYTE*)(tbl+0)=(BYTE)(0x41615252); *(BYTE*)((tbl+0)+1)=(BYTE)((WORD)(0x41615252)>>8); *(BYTE*)((tbl+0)+2)=(BYTE)((DWORD)(0x41615252)>>16); *(BYTE*)((tbl+0)+3)=(BYTE)((DWORD)(0x41615252)>>24);
		*(BYTE*)(tbl+484)=(BYTE)(0x61417272); *(BYTE*)((tbl+484)+1)=(BYTE)((WORD)(0x61417272)>>8); *(BYTE*)((tbl+484)+2)=(BYTE)((DWORD)(0x61417272)>>16); *(BYTE*)((tbl+484)+3)=(BYTE)((DWORD)(0x61417272)>>24);
		*(BYTE*)(tbl+488)=(BYTE)(n_clst - 1); *(BYTE*)((tbl+488)+1)=(BYTE)((WORD)(n_clst - 1)>>8); *(BYTE*)((tbl+488)+2)=(BYTE)((DWORD)(n_clst - 1)>>16); *(BYTE*)((tbl+488)+3)=(BYTE)((DWORD)(n_clst - 1)>>24);
		*(BYTE*)(tbl+492)=(BYTE)(0xFFFFFFFF); *(BYTE*)((tbl+492)+1)=(BYTE)((WORD)(0xFFFFFFFF)>>8); *(BYTE*)((tbl+492)+2)=(BYTE)((DWORD)(0xFFFFFFFF)>>16); *(BYTE*)((tbl+492)+3)=(BYTE)((DWORD)(0xFFFFFFFF)>>24);
		disk_write(drv, tbl, b_vol + 1, 1);	 
		disk_write(drv, tbl, b_vol + 7, 1);	 
	}

	return (disk_ioctl(drv, 0, (void*)0) == RES_OK) ? FR_OK : FR_DISK_ERR;
}






