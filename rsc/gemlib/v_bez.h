static inline void
_v_bez (short count, short *xyarr, char *bezarr, short *vdi_intin, short *vdi_ptsin)
{
	char  *opbd = (char*)vdi_intin;
	short *optr = vdi_ptsin;
	short *end;

	end = (short*)(bezarr + count);
	while (bezarr < (char*)end) {
		*(opbd +1) = *bezarr++;
		if (bezarr >= (char*)end)
		{
			*opbd = 0;
			break;
		}
		*opbd = *bezarr++;
		opbd += 2;
	}
	end = optr + (count *2);
	while (optr < end) {
		*(optr++) = *(xyarr++);
	}
}


