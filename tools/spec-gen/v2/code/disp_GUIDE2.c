uint8_t disp_GUIDE2(uint8_t batvol)
{
	return disp_guide_page(&guide_table[1], &disp_ui, batvol);
}