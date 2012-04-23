<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:s="http://www.w3.org/2001/sw/DataAccess/rf1/result" version="1.0" xmlns:php="http://php.net/xsl" exclude-result-prefixes="php">
  <!-- Red and White XSLT -->
  <xsl:variable name="BASEURL" select="$baseUrl"/>
  <xsl:variable name="PATH" select="$path"/>
  <xsl:variable name="thisPid" select="$collectionPid"/>
  <xsl:variable name="size" select="20"/>
  <xsl:variable name="page" select="$hitPage"/>
  <xsl:variable name="start" select="((number($page) - 1) * number($size)) + 1"/>
  <xsl:variable name="end" select="($start - 1) + number($size)"/>
  <xsl:variable name="cellsPerRow" select="4"/>
  <xsl:variable name="count" select="count(s:sparql/s:results/s:result)"/>

  <xsl:template match="/">
    <xsl:if test="$count>0">
      <xsl:call-template name="render_pager"/>
      <table cellpadding="3" cellspacing="3" width="90%">
        <xsl:apply-templates select="s:sparql/s:results"/>
      </table><br clear="all" />
      <xsl:call-template name="render_pager"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="s:sparql/s:results">
    <xsl:for-each select="s:result[position() mod $cellsPerRow = 1 and position()>=$start and position() &lt;=$end]">
      <tr>
        <xsl:apply-templates select=". | following-sibling::s:result[position() &lt; $cellsPerRow]"/>
      </tr>
    </xsl:for-each>
  </xsl:template>
   
  <xsl:template name="render_pager">
   <!-- start previous next -->
    <div class="item-list">
      <ul class="pager">
        <xsl:choose>
          <xsl:when test="$end >= $count and $start = 1">
            <xsl:value-of select="concat($start, '-', $count, ' of ', $count, '&#160;')"/><br />
          </xsl:when>
          <xsl:when test="$end >= $count">
            <xsl:value-of select="concat($start, '-', $count, ' of ', $count, '&#160;')"/><br />
            <li class="pager-previous">
              <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $thisPid, '/-/Collection/', $page - 1)"/>
                </xsl:attribute>
                &lt;Prev
              </a>
            </li>
          </xsl:when>
          <xsl:when test="$start = 1">
            <xsl:value-of select="concat($start, '-', $end, ' of ', $count, '&#160;')"/><br />
            <li class="pager-next">
              <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $thisPid, '/-/Collection/', $page + 1)"/>
                </xsl:attribute>
                Next>
              </a>
            </li>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="concat($start, '-', $end, ' of ', $count, '&#160;')"/><br />
            <li class="pager-previous">
              <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $thisPid, '/-/Collection/', $page - 1)"/>
                </xsl:attribute>
                &lt;Prev
              </a>&#160;
            </li>
            <li class="pager-next">
              <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $thisPid, '/-/Collection/', $page + 1)"/>
                </xsl:attribute>
                Next>
              </a>
            </li>
          </xsl:otherwise>
        </xsl:choose>
      </ul>
    </div>
    <!-- end previous next-->
  </xsl:template>

  <xsl:template match="s:result">
    <xsl:variable name='OBJECTURI' select="s:object/@uri"/>
    <xsl:variable name='CONTENTURI' select="s:content/@uri"/>
    <xsl:variable name='CONTENTMODEL' select="substring-after($CONTENTURI,'/')"/>
    <xsl:variable name='PID' select="substring-after($OBJECTURI,'/')"/>
    <xsl:variable name="newTitle" >
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="s:title"/>
        <xsl:with-param name="from" select="'_'"/>
        <xsl:with-param name="to" select="' '"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="cleanTitle" select="php:functionString('fedora_repository_urlencode_string', $newTitle)"/>
    
    <xsl:variable name="linkUrl">
      <xsl:choose>
        <xsl:when test="($CONTENTMODEL='islandora:collectionCModel')">
          <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $PID, '/-/collection')"/>
        </xsl:when>
        <xsl:otherwise>
        <!--the below is an example of going straight to a datastream instead of the details page.
  <xsl:value-of select="$BASEURL"/>/fedora/repository/<xsl:copy-of select="$PID"/>/OBJ/<xsl:value-of select="s:title"/>-->
          <xsl:value-of select="concat($BASEURL, '/fedora/repository/', $PID)"/>
        </xsl:otherwise>
     </xsl:choose>
     <xsl:value-of select="s:content"/>
    </xsl:variable>
    <td valign="top" width="25%">
     <a>
      <xsl:attribute name="href">
        <xsl:value-of select="$linkUrl"/>
      </xsl:attribute>
      <img>
        <xsl:attribute name="src"><xsl:value-of select="concat($BASEURL, '/fedora/repository/', $PID, '/TN')"/></xsl:attribute>
        <xsl:attribute name="alt"><xsl:value-of select="$newTitle" disable-output-escaping="yes"/></xsl:attribute>
      </img>
     </a><br clear="all" />
     <a>
      <xsl:attribute name="href"><xsl:value-of select="$linkUrl"/>
      </xsl:attribute>
      <xsl:value-of select="$newTitle" disable-output-escaping="yes" />
     </a>
    <!-- example of a url that would drill down to the details page if the url above went directly to a datastream
  <xsl:if test="($CONTENTMODEL!='islandora:collectionCModel')">
  <br />[[ <a>
  <xsl:attribute name="href">
  <xsl:value-of select="$BASEURL"/>/fedora/repository/<xsl:copy-of select="$PID"/>/-/<xsl:value-of select="$cleanTitle"/>
  </xsl:attribute>
  DETAILS
  </a> ]]
  </xsl:if>-->

    </td>
    <xsl:if test="(position() = last()) and (position() &lt; $cellsPerRow)">
      <xsl:call-template name="FillerCells">
        <xsl:with-param name="cellCount" select="$cellsPerRow - position()"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="FillerCells">
    <xsl:param name="cellCount"/>
    <td>&#160;</td>
    <xsl:if test="$cellCount > 1">
      <xsl:call-template name="FillerCells">
        <xsl:with-param name="cellCount" select="$cellCount - 1"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

  <xsl:template name="replace-string">
    <xsl:param name="text"/>
    <xsl:param name="from"/>
    <xsl:param name="to"/>

    <xsl:choose>
      <xsl:when test="contains($text, $from)">

      <xsl:variable name="before" select="substring-before($text, $from)"/>
      <xsl:variable name="after" select="substring-after($text, $from)"/>
      <xsl:variable name="prefix" select="concat($before, $to)"/>

      <xsl:value-of select="$before"/>
      <xsl:value-of select="$to"/>
      <xsl:call-template name="replace-string">
        <xsl:with-param name="text" select="$after"/>
        <xsl:with-param name="from" select="$from"/>
        <xsl:with-param name="to" select="$to"/>
      </xsl:call-template>
     </xsl:when>
     <xsl:otherwise>
      <xsl:value-of select="$text"/>
     </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
