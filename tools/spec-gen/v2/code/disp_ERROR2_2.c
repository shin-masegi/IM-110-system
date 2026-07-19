uint8_t disp_ERROR2_2(err_num num, uint8_t batvol)
{
	return disp_error_page2(&error2_table[1], &disp_ui, batvol);
}