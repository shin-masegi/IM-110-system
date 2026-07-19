uint8_t disp_GUIDE1(uint8_t batvol)
{
	return disp_guide_page(&guide_table[0], &disp_ui, batvol);
}