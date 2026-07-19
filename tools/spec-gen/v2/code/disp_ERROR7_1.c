uint8_t disp_ERROR7_1(err_num num, uint8_t batvol)
{
	return disp_error_page1(&error1_table[6], &disp_ui, (uint8_t)num, batvol);
}