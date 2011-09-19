<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
 <xsl:output method="xml" omit-xml-declaration="yes"/>
 <!-- refworks xslt -->
 <xsl:template match="/">
  <xsl:if test="/reference/jf">
   <h2>General Information</h2>
   <table cellpadding="2" cellspacing="2">
    <tr><td ><strong>Periodical:</strong></td><td><xsl:value-of select="/reference/jf"/></td></tr>
    <tr><td><strong>Abbreviation:</strong></td><td><xsl:value-of select="/reference/jo"/></td></tr>
    <tr><td><strong>Volume:</strong></td><td><xsl:value-of select="/reference/vo"/></td></tr>
    <tr><td><strong>Issue:</strong></td><td><xsl:value-of select="/reference/is"/></td></tr>
    <tr><td><strong>Publisher:</strong></td><td><xsl:value-of select="/reference/pb"/></td></tr>
    <tr><td><strong>Place of Publication:</strong></td><td><xsl:value-of select="/reference/pp"/></td></tr>
    <tr><td><strong>Edition:</strong></td><td><xsl:value-of select="/reference/ed"/></td></tr>
    <tr><td><strong>Year:</strong></td><td><xsl:value-of select="/reference/yr"/></td></tr>
    <tr><td><strong>Date:</strong></td><td><xsl:value-of select="/reference/fd"/></td></tr>
    <tr><td><strong>Start Page:</strong></td><td><xsl:value-of select="/reference/sp"/></td></tr>
    <tr><td><strong>Other Pages:</strong></td><td><xsl:value-of select="/reference/op"/></td></tr>
    <tr><td><strong>ISSN/ISBN:</strong></td><td><xsl:value-of select="/reference/sn"/></td></tr>
    <tr><td><strong>Language:</strong></td><td><xsl:value-of select="/reference/la"/></td></tr>
    <tr><td><strong>UL:</strong></td><td><xsl:value-of select="/reference/ul"/></td></tr>
   </table>
  </xsl:if>
  <xsl:if test="/reference/t1">
  <h2>Titles</h2>
  <ul>
   <xsl:for-each select="/reference/t1">
    <li><xsl:value-of select="."/></li>
   </xsl:for-each>
  </ul>
  </xsl:if>
  <xsl:if test="/reference/t2">
      <ul>
       <h3>Secondary Titles</h3>
    <xsl:for-each select="/reference/t2">
     <li><xsl:value-of select="."/></li>
    </xsl:for-each>
   </ul>
  </xsl:if>
  <xsl:if test="/reference/a1">
  <h2>Authors</h2>
  <ul>
   <xsl:for-each select="/reference/a1">
    <li><xsl:value-of select="."/></li>
   </xsl:for-each>
  </ul>
   </xsl:if>

   <xsl:if test="/reference/a2">
    <ul>
   <h3>Secondary Authors</h3>
   <xsl:for-each select="/reference/a2">
    <li><xsl:value-of select="."/></li>
   </xsl:for-each>
  </ul>
   </xsl:if>
  <xsl:if test="/reference/k1">
   <h2>Keywords</h2>
   <ul>
    <xsl:for-each select="/reference/k1">
     <li><xsl:value-of select="."/></li>
    </xsl:for-each>
   </ul>
  </xsl:if>
  <xsl:if test="/reference/ab">
  <h2>Abstract</h2>
   <xsl:for-each select="/reference/ab">
    <div><xsl:value-of select="."/> </div>
   </xsl:for-each>
  </xsl:if>
  <xsl:if test="/reference/no">
  <h2>Notes</h2>
  <xsl:for-each select="/reference/no">
   <div><xsl:value-of select="."/> </div>
  </xsl:for-each>
  </xsl:if>
  <xsl:variable name="ISSN">
  <xsl:value-of select="/reference/sn"/>
 </xsl:variable>
  <xsl:variable name="BASEURL">
    http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=<xsl:value-of select="/reference/sn/text()"/>&amp;date=<xsl:value-of select="/reference/yr/text()"/>&amp;volume=<xsl:value-of select="/reference/vo/text()"/>&amp;issue=<xsl:value-of select="/reference/is/text()"/>&amp;spage=<xsl:value-of select="/reference/sp/text()"/>&amp;atitle=<xsl:value-of select="/reference/t1"/>&amp;stitle=<xsl:value-of select="/reference/jf"/>
 </xsl:variable>
 <br />
 <xsl:if test="/reference/sn">
 <div><a>
 <xsl:attribute name="href"><xsl:value-of select="$BASEURL"/></xsl:attribute>
 <xsl:attribute name="target">_blank</xsl:attribute>
 <img src="http://asin1.its.unb.ca:8000/muse/images/getit4.gif"/> </a></div>
 </xsl:if>
 </xsl:template>
</xsl:stylesheet>