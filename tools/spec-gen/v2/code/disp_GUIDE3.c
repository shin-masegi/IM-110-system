uint8_t disp_GUIDE3(uint8_t batvol)
{
	return disp_guide_page(&guide_table[2], &disp_ui, batvol);
}