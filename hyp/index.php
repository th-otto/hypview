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
function submitUrl(url)
{
	var f = document.getElementById('hypviewform');
	f.method = 'get';
	f.enctype = "application/x-www-form-urlencoded";
	var fileinput = document.getElementById('file');
	var urlinput = document.getElementById('url');
	fileinput.disabled = true;
	var oldvalue = urlinput.value;
	urlinput.value = url;
	f.submit();
	fileinput.disabled = false;
	urlinput.value = oldvalue;
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

<noscript>
<p><span style="color:red">
<b>Your browser does not support JavaScript.</b>
<br />
Some features will not work without JavaScript enabled.
</span>
<br />
<br /></p>
</noscript>

<table>
<tr style="vertical-align: top;">
<td>
<fieldset>
Type in URL of a .HYP file (it must be remotely accessible from that URL<br />
for example <a href="hypview.cgi?url=/hyp/ataripf.hyp">http://www.tho-otto.de/hyp/ataripf.hyp</a>):
<br />
<input type="text" id="url" name="url" size="60" tabindex="1" style="margin-top: 1ex;" />
<input id="submiturl" style="background-color: #cccccc; font-weight: bold; visibility: hidden;" type="button" value="View" onclick="submitUrl(document.getElementById('url').value);" />
<noscript>
<div id="submitnoscript"><span><input type="submit" style="background-color: #cccccc; font-weight: bold;" value="View" /></span></div>
</noscript>
<script type="text/javascript">
document.getElementById('submiturl').style.visibility="visible";
</script>
</fieldset>
<div id="uploadbox" style="display:none;">
<br />
<b>OR</b><br />
<br />
<fieldset>
Choose a .HYP file for upload <br />
<input type="file" id="file" name="file" size="60" accept=".hyp,.HYP" style="margin-top: 1ex;" />
<input id="submitfile" style="background-color: #cccccc; font-weight: bold;" type="button" value="View" onclick="submitFile();" /><br />
</fieldset>
<br />
</div>
<script type="text/javascript">
document.getElementById('uploadbox').style.display="block";
</script>
<br />

<fieldset>
<table>
<tr>
<td>
<em>Show images</em><br />
</td>
<td>
<em>Show menu</em><br />
</td>
<td style="padding-left: 2ex">
<input type="checkbox" name="showstg" value="1" /> Show ST-Guide source<br />
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
<td colspan="2">
Output encoding:
</td>
<td>
<select id="output_charset" name="charset">
<option value="latin1"> latin1 (ISO-8859-1) (Western Europe)</option>
<option value="latin2"> latin2 (ISO-8859-2) (Central Europe)</option>
<!-- <option value="latin5"> latin5 (ISO-8859-5) (Cyrillic)</option> NYI -->
<option value="cp1250"> Windows 1250 (Central Europe)</option>
<!-- <option value="cp1251"> Windows 1251 (Cyrillic)</option> NYI -->
<option value="cp1252"> Windows 1252 (Western Europe)</option>
<option value="atarist"> Atari-ST (might not work on non-Atari Browser)</option>
<option value="utf8" selected="selected"> UTF-8 (Unicode, Worldwide)</option>
</select>
</td>
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

<?php
$hypdir = '/hyp';
echo "Some popular Hypertexts:\n";
echo "<ul>\n";
echo "<li><a href=\"javascript: submitUrl('$hypdir/tosde.hyp');\">Die Anleitung zum TOS (german)</a></li>\n";
echo "<li><a href=\"javascript: submitUrl('$hypdir/tosen.hyp');\">The documentation for TOS (english)</a></li>\n";
echo "</ul>\n";

function js_escape($string)
{
	$string = str_replace('&', '&amp;', $string);
	$string = str_replace('<', '&lt;', $string);
	$string = str_replace('>', '&gt;', $string);
	$string = str_replace('"', '\&quot;', $string);
	return $string;
}

if ($dir = opendir($_SERVER['DOCUMENT_ROOT'] . $hypdir))
{
	echo "Local files:\n\n";
	echo "<ul>\n";
	$files = array();
	while (false !== ($entry = readdir($dir))) {
		if ($entry == ".") continue;
		if ($entry == "..") continue;
		if (!fnmatch("*.hyp", $entry)) continue;
		$files[] = $entry;
    }
    sort($files);
    foreach ($files as $entry) {
    	echo '<li><a href="javascript: submitUrl(&quot;' . "$hypdir/" . js_escape($entry) . '&quot;);">' . htmlspecialchars($entry, ENT_QUOTES, 'UTF-8') . "</a></li>\n";
    }
 	closedir($dir);
	echo "</ul>\n";
}
?>

</div>

</body>
</html>
