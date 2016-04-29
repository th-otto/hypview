<!DOCTYPE html>
<html xml:lang="en" lang="en">
<head>
<meta http-equiv="content-type" content="text/html;charset=UTF-8" />
<meta charset="UTF-8" />
<title>libhyp home</title>
</head>
<body>
<h1 style="font-weight: bold;">
HYP View Web Service<span style="font-size: 13pt"> - provided by <a href="http://www.tho-otto.de/" target="_top">Thorsten Otto</a></span>
</h1>
<br/><br/>

<em>Want to browse .HYP files in an HTML browser?</em><br/>
<br/>

<table>
<tr style="vertical-align: top;">
<td>
Type in URL of a .HYP file (it must be remotely accessible from that URL<br>
for example <a href="/hypview/hypview.cgi?url=http://jaysoft.atari.org/docs/ataripf.hyp">http://jaysoft.atari.org/docs/ataripf.hyp</a>):
<br>
<form action=hypview.cgi>
<input type="text" name="url" size="60" tabindex=1 required style="margin-top: 1ex;"><br>
<br>

<table>
<tr>
<td>
<em>Show images</em><br>
</td>
<td>
Show menu<br>
</td>
</tr>
<tr>
<td>
<input type="radio" name="hideimages" value="0" checked="checked"> yes<br>
</td>
<td>
<input type="radio" name="hidemenu" value="0" checked="checked"> yes<br>
</td>
</tr>
<tr>
<td>
<input type="radio" name="hideimages" value="1"> no<br>
</td>
<td>
<input type="radio" name="hidemenu" value="1"> no<br>
</td>
</tr>
<tr><td>&nbsp;</td></tr>
<tr>
<td>
Output encoding:
</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="latin1"> latin1 (ISO-8859-1)
</td>
<td>
<input type="radio" name="charset" value="cp1252"> Windows 1252
</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="latin2"> latin2 (ISO-8859-2)
</td>
<td>
<input type="radio" name="charset" value="cp1250"> Windows 1250
</td>
</tr>
<tr>
<td>
<input type="radio" name="charset" value="utf8" checked="checked"> UTF8
</td>
</tr>
</table>

<br>
<div style="text-align: right;"><b>
<input style="background-color: #cccccc; font-weight: bold;" type="submit" value="Browse"></b>
</div>
</form>
</td>

<td style="border-spacing: 10px;">
Keyboard navigation (only when menu is on using HTML 4.x accesskey attribute):
<dl>
<dt><em>ALT+S</em>
<dd>Search (case sensitive)
<dt><em>ALT+P</em>
<dd>Previous node
<dt><em>ALT+N</em>
<dd>Next node
<dt><em>ALT+T</em>
<dd>Table of contents node
<dt><em>ALT+X</em>
<dd>Index node
<dt><em>ALT+O</em>
<dd>Open another .HYP file
</dl>
</td>
</tr>

</table>

</body>
</html>
