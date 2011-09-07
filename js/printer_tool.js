// $Id$ /**
/* Add printer-friendly tool to page. */
var PrinterTool = {};
PrinterTool.windowSettings = 'toolbar=no,location=no,' + 'status=no,menu=no,scrollbars=yes,width=650,height=400';
/** * Open a printer-friendly page and prompt for printing. * @param tagID *	The ID of the tag that contains the material that should *	be printed. */
PrinterTool.print = function (tagID) {
    var target = document.getElementById(tagID);
    var title = document.title;
    if(!target || target.childNodes.length === 0) {
        alert(t("Nothing to Print"));
        return;
    }
    var content = target.innerHTML;
    var text = '<html><head><title>' + title +
    '</title><body>' + content +'</body></html>';
    printerWindow = window.open('', '', PrinterTool.windowSettings);
    printerWindow.document.open();
    printerWindow.document.write(text);
    printerWindow.document.close();
    printerWindow.print();
};
