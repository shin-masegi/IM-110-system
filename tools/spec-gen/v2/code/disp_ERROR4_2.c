uint8_t disp_ERROR4_2(err_num num, uint8_t batvol)
{
	return disp_error_page2(&error2_table[3], &disp_ui, batvol);
}