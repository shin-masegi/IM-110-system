uint8_t disp_ERROR6_2(err_num num, uint8_t batvol)
{
	return disp_error_page2(&error2_table[5], &disp_ui, batvol);
}