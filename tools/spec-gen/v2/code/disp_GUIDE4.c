uint8_t disp_GUIDE4(uint8_t batvol)
{
	return disp_guide_page(&guide_table[3], &disp_ui, batvol);
}