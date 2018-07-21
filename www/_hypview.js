"use strict;"
var hypview_patterns = [
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAADEDgAAxA4AAAAAAAAAAAAAAAAAAP///wB3dwAA//8AAN3dAAD//wAAd3cAAP//AADd3QAA//8AAHd3AAD//wAA3d0AAP//AAB3dwAA//8AAN3dAAD//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wCqqgAA//8AAKqqAAD//wAAqqoAAP//AACqqgAA//8AAKqqAAD//wAAqqoAAP//AACqqgAA//8AAKqqAAD//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wCqqgAAd3cAAKqqAADd3QAAqqoAAHd3AACqqgAA3d0AAKqqAAB3dwAAqqoAAN3dAACqqgAAd3cAAKqqAADd3QAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBVVQAAqqoAAFVVAACqqgAAVVUAAKqqAABVVQAAqqoAAFVVAACqqgAAVVUAAKqqAABVVQAAqqoAAFVVAACqqgAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAREQAAqqoAAEREAACqqgAAEREAAKqqAABERAAAqqoAABERAACqqgAAREQAAKqqAAAREQAAqqoAAEREAACqqgAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAqqoAAAAAAACqqgAAAAAAAKqqAAAAAAAAqqoAAAAAAACqqgAAAAAAAKqqAAAAAAAAqqoAAAAAAACqqgAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAIiIAAAAAAACIiAAAAAAAACIiAAAAAAAAiIgAAAAAAAAiIgAAAAAAAIiIAAAAAAAAIiIAAAAAAACIiAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDv7wAA7+8AAO/vAAAAAAAA/v4AAP7+AAD+/gAAAAAAAO/vAADv7wAA7+8AAAAAAAD+/gAA/v4AAP7+AAAAAAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD39wAA7+8AANfXAAC7uwAAfX0AAP7+AAD9/QAA+/sAAPf3AADv7wAA19cAALu7AAB9fQAA/v4AAP39AAD7+wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wC+vgAAf38AAP//AAD//wAA6+sAAPf3AAD//wAA//8AAL6+AAB/fwAA//8AAP//AADr6wAA9/cAAP//AAD//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBfXwAAqqoAAPv7AAD7+wAA9fUAAKqqAAC/vwAAv78AAF9fAACqqgAA+/sAAPv7AAD19QAAqqoAAL+/AAC/vwAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD7+wAA//8AAL+/AADf3wAA7+8AAP//AAD+/gAA/f0AAPv7AAD//wAAv78AAN/fAADv7wAA//8AAP7+AAD9/QAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD/+QAAM88AAHJOAAB+fgAA5+cAAOTkAACcnAAAn5kAAP/5AAAzzwAAck4AAH5+AADn5wAA5OQAAJycAACfmQAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//4AAP//AAD3/wAA//8AAP/fAAD//wAA//8AAP//AAD//gAA//8AAPf/AAD//wAA/98AAP//AAD//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wBwcAAAOTkAAJOTAAAHBwAADg4AAJycAADJyQAA4OAAAHBwAAA5OQAAk5MAAAcHAAAODgAAnJwAAMnJAADg4AAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA7u4AAH19AAC7uwAA19cAAO7uAAD//wAAqqoAAP//AADu7gAAfX0AALu7AADX1wAA7u4AAP//AACqqgAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA7e4AAP//AADv7wAA//8AAKqqAAD//wAA7+8AAP//AADu7gAA//8AAO/vAAD//wAAqqoAAP//AADv7wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAODgAADg4AAG5uAAAREQAA4OAAAODgAADm5gAAEREAAA4OAAAODgAAbm4AABERAADg4AAA4OAAAObmAAAREQAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wA4OAAA19cAAO/vAADv7wAAg4MAAH19AAD+/gAA/v4AADg4AADX1wAA7+8AAO/vAACDgwAAfX0AAP7+AAD+/gAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAH9/AACfnwAA5+cAANvbAAC9vQAAfn4AAP7+AAD+/gAAf38AAJ+fAADn5wAA29sAAL29AAB+fgAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAPDwAADw8AAA8PAAAPDwAA8PAAAPDwAADw8AAA8PAAAA8PAAAPDwAADw8AAA8PAADw8AAA8PAAAPDwAADw8AAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDHxwAAg4MAAAEBAAAAAAAAAQEAAIODAADHxwAA7+8AAMfHAACDgwAAAQEAAAAAAAABAQAAg4MAAMfHAADv7wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wAAAAAAu7sAAN3dAADu7gAAAAAAAN3dAAC7uwAAd3cAAAAAAAC7uwAA3d0AAO7uAAAAAAAA3d0AALu7AAB3dwAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/f0AAPv7AAD39wAA7+8AAN/fAAC/vwAAf38AAP7+AAD9/QAA+/sAAPf3AADv7wAA398AAL+/AAB/fwAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDz8wAA5+cAAM/PAACfnwAAPz8AAH5+AAD8/AAA+fkAAPPzAADn5wAAz88AAJ+fAAA/PwAAfn4AAPz8AAD5+QAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wDb2wAA5+cAAOfnAADb2wAAvb0AAH5+AAB+fgAAvb0AANvbAADn5wAA5+cAANvbAAC9vQAAfn4AAH5+AAC9vQAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAAAAAAAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAAAAAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAAAAAAAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAAAAAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//gAA//0AAP/7AAD/9wAA/+8AAP/fAAD/vwAA/38AAP7/AAD9/wAA+/8AAPf/AADv/wAA3/8AAL//AAB//wAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wB//AAA//gAAP/xAAD/4wAA/8cAAP+PAAD/HwAA/j8AAPx/AAD4/wAA8f8AAOP/AADH/wAAj/8AAB//AAA//gAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wB//gAAv/0AAN/7AADv9wAA9+8AAPvfAAD9vwAA/n8AAP5/AAD9vwAA+98AAPfvAADv9wAA3/sAAL/9AAB//gAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA//4AAP/+AAD//gAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAD//wAA//8AAP//AAAAAAAA',
  'data:image/bmp;base64,Qk1+AAAAAAAAAD4AAAAoAAAAEAAAABAAAAABAAEAAAAAAEAAAAAAAAAAAAAAAAAAAAACAAAAAAAAAP///wD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAD+/gAAAAAAAP7+AAD+/gAA/v4AAP7+AAD+/gAA/v4AAP7+AAAAAAAA'
];
function showPopup (id) {
  var a = document.getElementById(id + '_content');
  if (a) {
    if (a.style.display == 'none' || a.style.display == '') {
      a.style.display = 'block';
    } else {
      a.style.display = 'none';
    }
  }
}
function hidePopup (id) {
  var a = document.getElementById(id + '_content');
  if (a)
    a.style.display = 'none';
}
function hideInfo () {
  var id = 'hyp_info';
  var a = document.getElementById(id + '_content');
  if (a)
    a.style.display = 'none';
}
function hideXrefs () {
  var id = 'hyp_xrefs';
  var a = document.getElementById(id + '_content');
  if (a)
    a.style.display = 'none';
}
function showInfo () {
  hideXrefs();
  var id = 'hyp_info';
  var a = document.getElementById(id + '_content');
  if (a) {
    if (a.style.display == 'none' || a.style.display == '') {
      a.style.display = 'inline-block';
    } else {
      a.style.display = 'none';
    }
  }
}
function showXrefs () {
  hideInfo();
  var id = 'hyp_xrefs';
  var a = document.getElementById(id + '_content');
  if (a) {
    if (a.style.display == 'none' || a.style.display == '') {
      a.style.display = 'inline-block';
    } else {
      a.style.display = 'none';
    }
  }
}
function getQueryVariable(variable)
{
  var query = window.location.search.substring(1);
  var vars = query.split('&');
  for (var i = 0; i < vars.length; i++) {
    var pair = vars[i].split('=');
    if (decodeURIComponent(pair[0]) == variable) {
      return decodeURIComponent(pair[1]);
    }
  }
  return '';
}
var lang;
var languages = ['en', 'de'];
function getSupportedLanguage(l)
{
  for (var j = 0; j < languages.length; j++) {
    if (languages[j] == l) return l;
  }
  return '';
}
function getAcceptLanguage()
{
  if (window.navigator.languages) {
    for (var i = 0; i < window.navigator.languages.length; i++) {
      var l = window.navigator.languages[i].split('-')[0].split('_')[0];
      l = getSupportedLanguage(l);
      if (l != '') return l;
    }
  }
  return '';
}
function getLanguage()
{
  var l = getQueryVariable('lang').split('-')[0].split('_')[0];
  if (l == '') {
    l = getAcceptLanguage();
  }
  l = getSupportedLanguage(l);
  if (l == '')
    l = getSupportedLanguage(navigator.language);
  if (l == '')
    l = 'en';
  lang = l;
  var html = document.getElementsByTagName('html')[0];
  html.setAttribute('lang', lang);
  html.setAttribute('xml:lang', lang);
}
getLanguage();
var xraster = 0;
var yraster = 0;
function getRaster()
{
  var d = document.getElementById('hypview_linetest');
  if (xraster == 0)
  {
    var d = document.createElement('div');
    var p = document.createElement('pre');
    var s = document.createElement('span');
    var t = document.createTextNode('y');
    s.appendChild(t);
    p.appendChild(s);
    d.appendChild(p);
    d.style.visibility = 'hidden';
    document.body.appendChild(d);
    xraster = s.offsetWidth;
    yraster = s.offsetHeight;
    document.body.removeChild(d);
  }
}
function drawLine(id, xoffset, width, height, begarrow, endarrow, linestyle)
{
  var c = document.getElementById(id);
  var ctx = c.getContext('2d');
  var style = window.getComputedStyle(c);
  ctx.font = style.getPropertyValue('font-size') + ' ' + style.getPropertyValue('font-family');
  getRaster();
  var x0, y0, x1, y1;
  if (width < 0)
  {
    xoffset += width;
    /* draw from right to left */
    width = (-width) * xraster;
    x0 = width + 0.5;
    x1 = 0.5;
  } else if (width == 0)
  {
    /* vertical line */
    width = 1;
    x0 = 0.5;
    x1 = 0.5;
  } else
  {
    /* draw from left to right */
    width = width * xraster;
    x0 = 0.5;
    x1 = width + 0.5;
  }
  if (height < 0)
  {
    /* draw from bottom to top */
    height = -height;
    height = height * (yraster + 0.05);
    y0 = height + 0.5;
    y1 = 0.5;
  } else if (height == 0)
  {
    /* horizontal line */
    height = 1;
    y0 = 0.5;
    y1 = 0.5;
  } else
  {
    /* draw from top to bottom */
    height = height * (yraster + 0.05);
    y0 = 0.5;
    y1 = height + 0.5;
  }
  c.width = width + 1;
  c.height = height + 1;
  c.style.left = ((xoffset - 1) * xraster).toString() + 'px';
  ctx.lineWidth = 1;
  ctx.shadowBlur = 0;
  ctx.filter = 'none';
  ctx.strokeStyle = style.getPropertyValue('color');
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.webkitImageSmoothingEnabled = false;
  ctx.msImageSmoothingEnabled = false;
  try {
    switch (linestyle)
    {
    default:
    case 1: /* SOLID */
      break;
    case 2: /* LONGDASH */
      ctx.setLineDash([12, 4]);
      break;
    case 3: /* DOT */
      ctx.setLineDash([2, 6, 2, 6]);
      break;
    case 4: /* DASHDOT */
      ctx.setLineDash([8, 3, 2, 3]);
      break;
    case 5: /* DASH */
      ctx.setLineDash([8, 8]);
      break;
    case 6: /* DASH2DOT */
      ctx.setLineDash([4, 3, 2, 2, 1, 3, 1, 0]);
      break;
    case 7: /* USERLINE */
      ctx.setLineDash([1, 1]);
      break;
    }
  } catch (e)
  {
    /* setLineDash not supported, but not much we can do about it */
  }
  ctx.beginPath();
  ctx.moveTo(x0, y0);
  ctx.lineTo(x1, y1);
  ctx.stroke();
}
function roundedBox(ctx, width, height)
{
  var deltax, deltay, xrad, yrad;
  rdeltax = width / 2;
  rdeltay = height / 2;
  xrad = 15;
  if (xrad > rdeltax)
    xrad = rdeltax;
  yrad = xrad;
  if (yrad > rdeltay)
    yrad = rdeltay;
  ctx.beginPath();
  ctx.moveTo(0.5, 0.5 + yrad);
  ctx.quadraticCurveTo(0.5, 0.5, 0.5 + xrad, 0.5);
  ctx.lineTo(width + 0.5 - xrad, 0.5);
  ctx.quadraticCurveTo(width + 0.5, 0.5, width + 0.5, 0.5 + yrad);
  ctx.lineTo(width + 0.5, height + 0.5 - yrad);
  ctx.quadraticCurveTo(width + 0.5, height + 0.5, width + 0.5 - xrad, height + 0.5);
  ctx.lineTo(0.5 + xrad, height + 0.5);
  ctx.quadraticCurveTo(0.5, height + 0.5, 0.5, height + 0.5 - yrad);
  ctx.lineTo(0.5, 0.5 + yrad);
}
function drawBox(id, xoffset, width, height, fillstyle, rounded)
{
  var c = document.getElementById(id);
  var ctx = c.getContext('2d');
  var style = window.getComputedStyle(c);
  ctx.font = style.getPropertyValue('font-size') + ' ' + style.getPropertyValue('font-family');
  getRaster();
  width = width * xraster;
  height = height * (yraster + 0.05);
  c.width = width + 1;
  c.height = height + 1;
  c.style.left = ((xoffset - 1) * xraster).toString() + 'px';
  ctx.lineWidth = 1;
  ctx.shadowBlur = 0;
  ctx.filter = 'none';
  ctx.strokeStyle = style.getPropertyValue('color');
  ctx.imageSmoothingEnabled = false;
  ctx.mozImageSmoothingEnabled = false;
  ctx.webkitImageSmoothingEnabled = false;
  ctx.msImageSmoothingEnabled = false;
  if (rounded)
  {
    roundedBox(ctx, width, height);
  } else
  {
    ctx.rect(0.5, 0.5, width, height);
  }
  if (fillstyle != 0)
  {
    if ((fillstyle >= 1 && fillstyle <= 7) || (fillstyle >= 9 && fillstyle <= 36))
    {
      var img;
      img = new Image();
      img.src = hypview_patterns[fillstyle];
      img.onload = function() {
        ctx.fillStyle = ctx.createPattern(img, 'repeat');
        ctx.fill();
        if (fillstyle != 8)
        {
          ctx.stroke();
        }
      };
      return;
    } else
    {
      ctx.fill();
    }
  }
  if (fillstyle != 8)
  {
    ctx.stroke();
  }
}
function execSystem(title, cmd)
{
  window.confirm(title + ' ' + cmd);
}
function execRx(title, cmd)
{
  window.confirm(title + ' ' + cmd);
}
function execRxs(title, cmd)
{
  window.confirm(title + ' ' + cmd);
}
