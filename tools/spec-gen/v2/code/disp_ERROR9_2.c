uint8_t disp_ERROR9_2(err_num num, uint8_t batvol)
{
	return disp_error_page2(&error2_table[8], &disp_ui, batvol);
}