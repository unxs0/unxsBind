function open_popup(page)
{
	window_handle = window.open(page,'Glossary','height=600,width=500,status=yes,toolbar=no,menubar=no,location=no,scrollbars=1');
	window_handle.focus();
	return false;

}//function open_popup(page)

