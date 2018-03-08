void *hfix_objs(RSHDR *hdr, OBJECT *tree, _WORD num_objs)
{
	_WORD i;
	_WORD num_user;
	
	num_user = 0;
	for (i = 0; i < num_objs; i++)
	{
		OBJECT *ob = &tree[i];
		
		switch (ob->ob_type & OBTYPEMASK)
		{
		case G_ICON:
			{
				ICONBLK *ib;

				ib = ob->ob_spec.iconblk;
				W_Fix_Bitmap(ib->ib_pdata, ib->ib_wicon, ib->ib_hicon, 1);
				W_Fix_Bitmap(ib->ib_pmask, ib->ib_wicon, ib->ib_hicon, 1);
			}
			break;

		case G_CICON:
			{
				CICONBLK *cicon = ob->ob_spec.ciconblk;
				CICON *cic;

				W_Fix_Bitmap(cicon->monoblk.ib_pdata, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
				W_Fix_Bitmap(cicon->monoblk.ib_pmask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
				cic = cicon->mainlist;
				while (cic != NULL)
				{
					if (!W_Fix_Bitmap(cic->col_data, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, cic->num_planes))
					{
						ob->ob_type = (ob->ob_type & OBEXTTYPEMASK) | G_ICON;
						ob->ob_spec.iconblk = &cicon->monoblk;
						break;
					} else
					{
						W_Fix_Bitmap(cic->col_mask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
						if (cic->sel_data)
						{
							W_Fix_Bitmap(cic->sel_data, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, cic->num_planes);
							W_Fix_Bitmap(cic->sel_mask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
						}
						cic = cic->next_res;
					}
				}
				ob->ob_spec.iconblk = &cicon->monoblk;
				ob->ob_type = (ob->ob_type & OBEXTTYPEMASK) | G_ICON;
			}
			break;

		case G_IMAGE:
			{
				BITBLK *bi;

				bi = ob->ob_spec.bitblk;
				W_Fix_Bitmap(bi->bi_pdata, bi->bi_wb * 8, bi->bi_hl, 1);
			}
			break;

		case G_FTEXT:
		case G_FBOXTEXT:
			break;

		case G_TEXT:
		case G_BOXTEXT:
			break;

		case G_STRING:
		case G_TITLE:
		case G_BUTTON:
			break;
		}
	}
}

/*** ---------------------------------------------------------------------- ***/

void hrelease_objs(OBJECT *tree, _WORD num_objs)
{
	_WORD i;
	
	for (i = 0; i < num_objs; i++)
	{
		OBJECT *ob = &tree[i];
		
		switch (ob->ob_type & OBTYPEMASK)
		{
		case G_ICON:
			{
				ICONBLK *ib;

				ib = ob->ob_spec.iconblk;
				if (ib->ib_pdata != NULL)
				{
					W_Release_Bitmap(ib->ib_pdata, ib->ib_wicon, ib->ib_hicon, 1);
					W_Release_Bitmap(ib->ib_pmask, ib->ib_wicon, ib->ib_hicon, 1);
				}
			}
			break;

		case G_CICON:
			{
				CICONBLK *cicon = ob->ob_spec.ciconblk;
				CICON *cic;

				if (cicon->monoblk.ib_pdata != NULL)
				{
					W_Release_Bitmap(cicon->monoblk.ib_pdata, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
					W_Release_Bitmap(cicon->monoblk.ib_pmask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
					cic = cicon->mainlist;
					while (cic != NULL)
					{
						W_Release_Bitmap(cic->col_data, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, cic->num_planes);
						W_Release_Bitmap(cic->col_mask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
						if (cic->sel_data)
						{
							W_Release_Bitmap(cic->sel_data, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, cic->num_planes);
							W_Release_Bitmap(cic->sel_mask, cicon->monoblk.ib_wicon, cicon->monoblk.ib_hicon, 1);
						}
						cic = cic->next_res;
					}
				}
			}
			break;

		case G_IMAGE:
			{
				BITBLK *bi;

				bi = ob->ob_spec.bitblk;
				if (bi->bi_pdata != NULL)
				{
					W_Release_Bitmap(bi->bi_pdata, bi->bi_wb * 8, bi->bi_hl, 1);
				}
			}
			break;

		}
	}

}
