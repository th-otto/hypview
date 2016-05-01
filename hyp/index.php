<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN"
          "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xml:lang="en" lang="en" xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="content-type" content="text/html;charset=UTF-8" />
<title>libhyp home</title>
<style type="text/css">
body {
	margin:1em;
	padding:1em;
	background-color: #cccccc;
}
</style>
<script type="text/javascript">
function submitUrl()
{
	var f = document.getElementById('hypviewform');
	f.method = 'get';
	f.enctype = "application/x-www-form-urlencoded";
	var fileinput = document.getElementById('file');
	fileinput.disabled = true;
	f.submit();
	fileinput.disabled = false;
}
function submitFile()
{
	var f = document.getElementById('hypviewform');
	f.method = 'post';
	f.enctype = "multipart/form-data";
	f.submit();
}
</script>
</head>
<body>
<div>
<h1 style="font-weight: bold;">
HYP View Web Service<span style="font-size: 13pt"> - provided by <a href="http://www.tho-otto.de/">Thorsten Otto</a></span>
</h1>
<br/><br/>

<em>Want to browse .HYP files in an HTML browser?</em><br />
<br />

<form action="hypview.cgi" method="get" id="hypviewform">
<table>
<tr style="vertical-align: top;">
<td>
<fieldset>
Type in URL of a .HYP file (it must be remotely accessible from that URL<br />
for example <a href="/hypview/hypview.cgi?url=http://jaysoft.atari.org/docs/ataripf.hyp">http://jaysoft.atari.org/docs/ataripf.hyp</a>):
<br />
<input type="text" id="url" name="url" size="60" tabindex="1" style="margin-top: 1ex;" />
<input style="background-color: #cccccc; font-weight: bold;" type="button" value="View" onclick="submitUrl();" /><br />
</fieldset>
<br />
<b>OR</b><br />
<br />
<fieldset>
Choose a .HYP file for upload <br />
<input type="file" id="file" name="file" size="60" style="margin-top: 1ex;" />
<input style="background-color: #cccccc; font-weight: bold;" type="button" value="View" onclick="submitFile();" /><br />
</fieldset>
<br />
<br />

<fieldset>
<table>
<tr>
<td>
<em>Show images</em><br />
</td>
<td>
Show menu<br />
</td>
</tr>
<tr>
<td>
<input type="radio" name="hideimages" value="0" checked="checked" /> yes<br />
</td>
<td>
<input type="radio" name="hidemenu" value="0" checked="checked" /> yes<br />
</td>
</tr>
<tr>
<td>
<input type="radio" name="hideimages" value="1" /> no<br />
</td>
<td>
<input type="radio" name="hidemenu" value="1" /> no<br />
</td>
</tr>
<tr><td>&nbsp;</td><td>&nbsp;</td></tr>
<tr>
<td>
Output encoding:
</td>
<td>&nbsp;</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="latin1" /> latin1 (ISO-8859-1)
</td>
<td>
<input type="radio" name="charset" value="cp1252" /> Windows 1252
</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="latin2" /> latin2 (ISO-8859-2)
</td>
<td>
<input type="radio" name="charset" value="cp1250" /> Windows 1250
</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="utf8" checked="checked" /> UTF8
</td>
<td>&nbsp;</td>
</tr>
</table>
</fieldset>

<br />
</td>

<td style="width:1ex;"> </td>
<td style="width:1ex; background-color: #888888;"> </td>
<td style="width:1ex;"> </td>

<td style="border-spacing: 10px;">
Keyboard navigation (only when menu is on using HTML 4.x accesskey attribute):
<dl>
<dt><em>ALT+S</em></dt>
<dd>Search (case sensitive)</dd>
<dt><em>ALT+P</em></dt>
<dd>Previous node</dd>
<dt><em>ALT+N</em></dt>
<dd>Next node</dd>
<dt><em>ALT+T</em></dt>
<dd>Table of contents node</dd>
<dt><em>ALT+X</em></dt>
<dd>Index node</dd>
<dt><em>ALT+O</em></dt>
<dd>Open another .HYP file</dd>
</dl>
</td>
</tr>

</table>
</form>

</div>
</body>
</html>
