uint8_t disp_ERROR3_2(err_num num, uint8_t batvol)
{
	return disp_error_page2(&error2_table[2], &disp_ui, batvol);
}