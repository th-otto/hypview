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
<!-- BEGIN CONFIGURATION -->
<?php
$hypdir = '/hyp';
?>
<script type="text/javascript">
var hypdir = '/hyp';
var hyptestdir = '/hyp/tests/';
</script>
<!-- END CONFIGURATION -->
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
$languages = array(
	'af' => array('name' => 'Afrikaans', 'flag' => 'za-af.gif'),
	'am' => array('name' => 'Amharic', 'flag' => 'et_h1.gif'),
	'an' => array('name' => 'Aragonese', 'flag' => 'es-ar.gif'),
	'ar' => array('name' => 'Arabic', 'flag' => 'sa.gif'),
	'as' => array('name' => 'Assamese', 'flag' => 'in-as.gif'),
	'az' => array('name' => 'Azeri', 'flag' => 'az.gif'),
	'be' => array('name' => 'Belarusian', 'flag' => 'by.gif'),
	'bg' => array('name' => 'Bulgarian', 'flag' => 'bg.gif'),
	'bn' => array('name' => 'Bengali', 'flag' => 'in-bn.gif'),
	'br' => array('name' => 'Breton', 'flag' => 'fr-bz.gif'),
	'bs' => array('name' => 'Bosnian', 'flag' => 'ba.gif'),
	'ca' => array('name' => 'Catalan', 'flag' => 'es-ct.gif'),
	'cs' => array('name' => 'Czech', 'flag' => 'cz.gif'),
	'cy' => array('name' => 'Welsh', 'flag' => 'cy.gif'),
	'da' => array('name' => 'Danish', 'flag' => 'dk.gif'),
	'de' => array('name' => 'German', 'flag' => 'de.gif'),
	'dz' => array('name' => 'Dzongkha', 'flag' => 'bt.gif'),
	'el' => array('name' => 'Greek', 'flag' => 'gr.gif'),
	'en' => array('name' => 'English', 'flag' => 'uk.gif'),
	'eo' => array('name' => 'Esperanto', 'flag' => 'qy-eo.gif'),
	'es' => array('name' => 'Spanish', 'flag' => 'es.gif'),
	'et' => array('name' => 'Estonian', 'flag' => 'ee.gif'),
	'eu' => array('name' => 'Basque', 'flag' => 'es-pv.gif'),
	'fa' => array('name' => 'Farsi', 'flag' => 'ir.gif'),
	'fi' => array('name' => 'Finnish', 'flag' => 'fi.gif'),
	'fo' => array('name' => 'Faroese', 'flag' => 'fo.gif'),
	'fr' => array('name' => 'French', 'flag' => 'fr.gif'),
	'ga' => array('name' => 'Irish', 'flag' => 'ie.gif'),
	'gl' => array('name' => 'Galician', 'flag' => 'es-ga.gif'),
	'gu' => array('name' => 'Gujarati', 'flag' => 'gu.gif'),
	'he' => array('name' => 'Hebrew', 'flag' => 'il.gif'),
	'hi' => array('name' => 'Hindi', 'flag' => 'in.gif'),
	'hr' => array('name' => 'Croatian', 'flag' => 'hr.gif'),
	'ht' => array('name' => 'Haitian', 'flag' => 'ht.gif'),
	'hu' => array('name' => 'Hungarian', 'flag' => 'hu.gif'),
	'hy' => array('name' => 'Armenian', 'flag' => 'am.gif'),
	'id' => array('name' => 'Indonesian', 'flag' => 'id.gif'),
	'is' => array('name' => 'Icelandic', 'flag' => 'is.gif'),
	'it' => array('name' => 'Italian', 'flag' => 'it.gif'),
	'ja' => array('name' => 'Japanese', 'flag' => 'jp.gif'),
	'jv' => array('name' => 'Javanese', 'flag' => 'jv.png'),
	'ka' => array('name' => 'Georgian', 'flag' => 'ge.gif'),
	'kk' => array('name' => 'Kazakh', 'flag' => 'kz.gif'),
	'km' => array('name' => 'Khmer', 'flag' => 'kh.gif'),
	'kn' => array('name' => 'Kannada', 'flag' => 'kn.png'),
	'ko' => array('name' => 'Korean', 'flag' => 'kr.gif'),
	'ku' => array('name' => 'Kurdish', 'flag' => 'ku.png'),
	'ky' => array('name' => 'Kyrgyz', 'flag' => 'kg.gif'),
	'la' => array('name' => 'Latin', 'flag' => 'va.gif'),
	'lb' => array('name' => 'Luxembourgish', 'flag' => 'lu.gif'),
	'lo' => array('name' => 'Lao', 'flag' => 'la.gif'),
	'lt' => array('name' => 'Lithuanian', 'flag' => 'lt.gif'),
	'lv' => array('name' => 'Latvian', 'flag' => 'lv.gif'),
	'mg' => array('name' => 'Malagasy', 'flag' => 'mg.gif'),
	'mk' => array('name' => 'Macedonian', 'flag' => 'mk.gif'),
	'ml' => array('name' => 'Malayalam', 'flag' => 'ml.png'),
	'mn' => array('name' => 'Mongolian', 'flag' => 'mn.gif'),
	'mr' => array('name' => 'Marathi', 'flag' => 'mr.png'),
	'ms' => array('name' => 'Malay', 'flag' => 'my.gif'),
	'mt' => array('name' => 'Maltese', 'flag' => 'mt.gif'),
	'nb' => array('name' => "Bokm\xc3\xa5l", 'flag' => 'no.gif'),
	'ne' => array('name' => 'Nepali', 'flag' => 'np.gif'),
	'nl' => array('name' => 'Dutch', 'flag' => 'nl.gif'),
	'nn' => array('name' => 'Nynorsk', 'flag' => 'no.gif'),
	'no' => array('name' => 'Norwegian', 'flag' => 'no.gif'),
	'oc' => array('name' => 'Occitan', 'flag' => 'oc.png'),
	'or' => array('name' => 'Oriya', 'flag' => 'or.png'),
	'pa' => array('name' => 'Punjabi', 'flag' => 'pa.png'),
	'pl' => array('name' => 'Polish', 'flag' => 'pl.gif'),
	'ps' => array('name' => 'Pashto', 'flag' => 'af.gif'),
	'pt' => array('name' => 'Portuguese', 'flag' => 'pt.gif'),
	'qu' => array('name' => 'Quechua', 'flag' => 'bo.gif'),
	'ro' => array('name' => 'Romanian', 'flag' => 'ro.gif'),
	'ru' => array('name' => 'Russian', 'flag' => 'ru.gif'),
	'rw' => array('name' => 'Kinyarwanda', 'flag' => 'rw.gif'),
	'se' => array('name' => 'Sami', 'flag' => 'se.png'),
	'si' => array('name' => 'Sinhalese', 'flag' => 'si-lk.gif'),
	'sk' => array('name' => 'Slovak', 'flag' => 'sk.gif'),
	'sl' => array('name' => 'Slovenian', 'flag' => 'sl.gif'),
	'sq' => array('name' => 'Albanian', 'flag' => 'al.gif'),
	'sr' => array('name' => 'Serbian', 'flag' => 'rs.gif'),
	'sv' => array('name' => 'Swedish', 'flag' => 'se.gif'),
	'sw' => array('name' => 'Swahili', 'flag' => 'ke.gif'),
	'ta' => array('name' => 'Tamil', 'flag' => 'ta-lk.png'),
	'te' => array('name' => 'Telugu', 'flag' => 'te.png'),
	'th' => array('name' => 'Thai', 'flag' => 'th.gif'),
	'tl' => array('name' => 'Tagalog', 'flag' => 'ph.gif'),
	'tr' => array('name' => 'Turkish', 'flag' => 'tr.gif'),
	'ug' => array('name' => 'Uighur', 'flag' => 'ug-cn.png'),
	'uk' => array('name' => 'Ukrainian', 'flag' => 'ua.gif'),
	'ur' => array('name' => 'Urdu', 'flag' => 'pk.gif'),
	'vi' => array('name' => 'Vietnamese', 'flag' => 'vn.gif'),
	'vo' => array('name' => "Volap\xc3\xbck", 'flag' => 'vo.png'),
	'wa' => array('name' => 'Walloon', 'flag' => 'be.gif'),
	'xh' => array('name' => 'Xhosa', 'flag' => 'xh-za.png'),
	'zh' => array('name' => 'Chinese', 'flag' => 'cn.gif'),
	'zu' => array('name' => 'Zulu', 'flag' => 'zu-za.png'),
);

echo "Some popular Hypertexts:\n";
echo "<ul>\n";
echo "<li><a href=\"javascript: submitUrl('$hypdir/tosde.hyp');\">Die Anleitung zum TOS (" . $languages['de']['name'] . ")</a>";
echo '<img alt="" width="32" height="21" src="images/flags/' . $languages['de']['flag'] . '">' . "\n";
echo "</li>\n";
echo "<li><a href=\"javascript: submitUrl('$hypdir/tosen.hyp');\">The documentation for TOS (" . $languages['en']['name'] . ")</a>";
echo '<img alt="" width="32" height="21" src="images/flags/' . $languages['en']['flag'] . '">' . "\n";
echo "</li>\n";
echo "</ul>\n";

function js_escape($string)
{
	$string = str_replace('&', '&amp;', $string);
	$string = str_replace('<', '&lt;', $string);
	$string = str_replace('>', '&gt;', $string);
	$string = str_replace('"', '\&quot;', $string);
	return $string;
}

class hyp {
	public $files;
	public function hyp($opt) {
		$this->files = array();
	}
	public function __destruct()
	{
	}

	public function cmp_name($a, $b)
	{
		if ($this->files[$a]['name'] > $this->files[$b]['name'])
			return 1;
		if ($this->files[$a]['name'] < $this->files[$b]['name'])
			return -1;
		return $a > $b;
	}

}

$localdir = $_SERVER['DOCUMENT_ROOT'] . $hypdir;

$cachefile = $localdir . '/hypinfo.cache';
$filelist = file_get_contents($cachefile);
if ($filelist === false)
{
	$filelist = '';
	if ($dir = opendir($localdir))
	{
		$filelist .= "Local files:\n\n";
		$hyp = new hyp(array());
		while (false !== ($entry = readdir($dir))) {
			if ($entry == ".") continue;
			if ($entry == "..") continue;
			if (!fnmatch("*.hyp", $entry)) continue;
			/* skip subsidiary files from collections (e.g. folder.hyp) */
			if (fnmatch("_*.hyp", $entry)) continue;
			/* skip the output result from hypfind tool */
			if (fnmatch("hypfind.hyp", $entry)) continue;
	
			$hyp->files[$entry] = array();
			$hyp->files[$entry]['name'] = $entry;
			$info = popen('./hypinfo ' . escapeshellarg($localdir . '/' . $entry), "r");
			if (is_resource($info))
			{
				while (($line = fgets($info)) !== false)
				{
					$line = rtrim($line, "\x0D\x0A");
					if (preg_match('/^@([a-z]+): (.*)/', $line, $matches))
						$hyp->files[$entry][$matches[1]] = $matches[2];
					if (preg_match('/^:([a-z]+): (.*)/', $line, $matches))
						$hyp->files[$entry][$matches[1]] = $matches[2];
				}
				pclose($info);
			}
	    }
	
	    uksort($hyp->files, array($hyp, 'cmp_name'));
		$filelist .= "<ul>\n";
		$filelist .= "<table>\n";
	    foreach ($hyp->files as $name => $entry) {
			$filelist .= "<tr>\n";
			$filelist .= "<td>\n";
	    	$filelist .= '<li><a href="javascript: submitUrl(&quot;' . "$hypdir/" . js_escape($name) . '&quot;);">' . htmlspecialchars($name, ENT_QUOTES, 'UTF-8') . "</a></li>\n";
			$filelist .= "</td>\n";
			$filelist .= "<td width=\"300\">\n";
			if (isset($entry['database']))
				$filelist .= htmlspecialchars($entry['database']);
			else if (isset($entry['subject']))
				$filelist .= htmlspecialchars($entry['subject']);
			$filelist .= "</td>\n";
			$filelist .= "<td>\n";
			$filelist .= htmlspecialchars(isset($entry['version']) ? $entry['version'] : '');
			$filelist .= "</td>\n";
			$filelist .= "<td>\n";
			if (isset($entry['lang']))
			{
				// if ($entry['lang'] == 'de') $entry['lang'] = 'nb';
				if (isset($languages[$entry['lang']]['name']))
					$filelist .= htmlspecialchars($languages[$entry['lang']]['name']);
				else
					$filelist .= htmlspecialchars($entry['lang']);
				$filelist .= "\n";
			}
			$filelist .= "</td>\n";
			if (isset($entry['lang']) && isset($languages[$entry['lang']]['flag']))
			{
				$filelist .= "<td>\n";
				$filelist .= '<img alt="" width="32" height="21" src="images/flags/' . $languages[$entry['lang']]['flag'] . '">' . "\n";
				$filelist .= "</td>\n";
			}
			$filelist .= "</tr>\n";
	    }
	 	closedir($dir);
		$filelist .= "</table>\n";
		$filelist .= "</ul>\n";
	}
	file_put_contents($cachefile, $filelist);
}
echo $filelist;
?>

Some files i used for testing:

<ul>
<table>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;b&lt;a&gt;d   &amp;f*i?l%e:n'a\&quot;m@e.hyp&quot;);">b&lt;a&gt;d   &amp;f*i?l%e:n&#039;a&quot;m@e.hyp</a></li>
</td>
<td>
Just a very strange filename, for testing of proper quoting
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;empty.hyp&quot;);">empty.hyp</a></li>
</td>
<td>
A file without any nodes (something that the compiler refuses to create)
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;image.hyp&quot;);">image.hyp</a></li>
</td>
<td>
A file with some @images
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;limage.hyp&quot;);">limage.hyp</a></li>
</td>
<td>
A file with some @limages
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;linkattr.hyp&quot;);">linkattr.hyp</a></li>
</td>
<td>
A file with all kinds of supported links
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;lines.hyp&quot;);">lines.hyp</a></li>
</td>
<td>
A file with all kinds of supported line drawings
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;textattr.hyp&quot;);">textattr.hyp</a></li>
</td>
<td>
A file with all kinds of supported text attributes
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;colors.hyp&quot;);">colors.hyp</a></li>
</td>
<td>
A file with text colors
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;patterns.hyp&quot;);">patterns.hyp</a></li>
</td>
<td>
A file with several fill patterns
</td>
</tr>

<tr>
<td>
<li><a href="javascript: submitUrl(hyptestdir + &quot;chartab.hyp&quot;);">chartab.hyp</a></li>
</td>
<td>
Atari character code table
</td>
</tr>

</table>
</ul>

<hr />
<div style="text-align:center">
<p style="font-size: 60%">Images of flags are from
<a href="http://flagspot.net/flags/"><img src="images/fotwlink.gif" width="45" height="30" style="border:0" alt="FOTW"> Flags Of The World</a>
</p>
</div>

</div>

</body>
</html>
