<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
 <!-- Red and White XSLT -->
    <xsl:variable name="BASEURL">
        <xsl:value-of select="$baseUrl"/>
    </xsl:variable>
    <xsl:variable name="PATH">
        <xsl:value-of select="$path"/>
    </xsl:variable>
    <xsl:variable name="thisPid" select="$collectionPid"/>
    <xsl:variable name="thisTitle" select="$collectionTitle"/>
    <xsl:variable name="size"  select="20"/>
    <xsl:variable name="page" select="$hitPage"/>
    <xsl:variable name="start" select="((number($page) - 1) * number($size)) + 1"/>
    <xsl:variable name="end" select="($start - 1) + number($size)"/>
    <xsl:variable name="cellsPerRow" select="4"/>
    <xsl:variable name="count" select="count(sparql/results/result)"/>
    <xsl:template match="/">
        <xsl:if test="$count>0">
            <table cellpadding="3" cellspacing="3" width="90%">
                <tr>
                    <td colspan="{$cellsPerRow}">
                        <div STYLE="text-align: center;">
                            <xsl:choose>
                                <xsl:when test="$end >= $count and $start = 1">
                                    <xsl:value-of select="$start"/>-
                                    <xsl:value-of select="$count"/>
     of 
                                    <xsl:value-of select="$count"/>&#160;
                                    <br />
                                </xsl:when>
                                <xsl:when test="$end >= $count">
                                    <xsl:value-of select="$start"/>-
                                    <xsl:value-of select="$count"/>
     of 
                                    <xsl:value-of select="$count"/>&#160;
                                    <br />
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                            <xsl:value-of select="$thisPid"/>/-/
                                            <xsl:value-of select="$thisTitle"/>/
                                            <xsl:value-of select="$page - 1"/>
                                        </xsl:attribute>
      &lt;&lt;Prev
                                    </a>
                                </xsl:when>
                                <xsl:when test="$start = 1">
                                    <xsl:value-of select="$start"/>-
                                    <xsl:value-of select="$end"/>
     of 
                                    <xsl:value-of select="$count"/>&#160;
                                    <br />
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                            <xsl:value-of select="$thisPid"/>/-/
                                            <xsl:value-of select="$thisTitle"/>/
                                            <xsl:value-of select="$page + 1"/>
                                        </xsl:attribute>
      Next>>
                                    </a>
                                </xsl:when>
                                <xsl:otherwise>
                                    <xsl:value-of select="$start"/>-
                                    <xsl:value-of select="$end"/>
     of 
                                    <xsl:value-of select="$count"/>&#160;
                                    <br />
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                            <xsl:value-of select="$thisPid"/>/-/
                                            <xsl:value-of select="$thisTitle"/>/
                                            <xsl:value-of select="$page - 1"/>
                                        </xsl:attribute>
      &lt;&lt;Prev
                                    </a>&#160;
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                            <xsl:value-of select="$thisPid"/>/-/
                                            <xsl:value-of select="$thisTitle"/>/
                                            <xsl:value-of select="$page + 1"/>
                                        </xsl:attribute>
      Next>>
                                    </a>
                                </xsl:otherwise>
                            </xsl:choose>
                        </div>
                        <br clear="all" />
                    </td>
                </tr>

  <!--<xsl:for-each select="/sparql/results/result[position()>=$start and position() &lt;=$end]">
  <xsl:variable name='OBJECTURI' select="object/@uri"/>
  <xsl:variable name='PID' select="substring-after($OBJECTURI,'/')"/>
   <tr>
  <td>
    <img>
     <xsl:attribute name="src"><xsl:value-of select="$BASEURL"/>/fedora/repository/<xsl:value-of select="$PID"/>/TN
     </xsl:attribute>
    </img>
    <a>
     <xsl:attribute name="href"><xsl:value-of select="$BASEURL"/>/fedora/repository/<xsl:copy-of select="$PID"/>/-/<xsl:value-of select="title"/>
     </xsl:attribute>
    <xsl:value-of select="title"/>
   </a>
  </td>
   </tr>
  </xsl:for-each>-
 -->
                <xsl:apply-templates select="sparql/results"/>
            </table>
            <br clear="all" />
            <div STYLE="text-align: center;">
                <xsl:choose>
                    <xsl:when test="$end >= $count and $start = 1">
                        <xsl:value-of select="$start"/>-
                        <xsl:value-of select="$count"/>
   of 
                        <xsl:value-of select="$count"/>&#160;
                        <br />
                    </xsl:when>
                    <xsl:when test="$end >= $count">
                        <xsl:value-of select="$start"/>-
                        <xsl:value-of select="$count"/>
   of 
                        <xsl:value-of select="$count"/>&#160;
                        <br />
                        <a>
                            <xsl:attribute name="href">
                                <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                <xsl:value-of select="$thisPid"/>/-/
                                <xsl:value-of select="$thisTitle"/>/
                                <xsl:value-of select="$page - 1"/>
                            </xsl:attribute>
    &lt;&lt;Prev
                        </a>
                    </xsl:when>
                    <xsl:when test="$start = 1">
                        <xsl:value-of select="$start"/>-
                        <xsl:value-of select="$end"/>
   of 
                        <xsl:value-of select="$count"/>&#160;
                        <br />
                        <a>
                            <xsl:attribute name="href">
                                <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                <xsl:value-of select="$thisPid"/>/-/
                                <xsl:value-of select="$thisTitle"/>/
                                <xsl:value-of select="$page + 1"/>
                            </xsl:attribute>
    Next>>
                        </a>
                    </xsl:when>
                    <xsl:otherwise>
                        <xsl:value-of select="$start"/>-
                        <xsl:value-of select="$end"/>
   of 
                        <xsl:value-of select="$count"/>&#160;
                        <br />
                        <a>
                            <xsl:attribute name="href">
                                <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                <xsl:value-of select="$thisPid"/>/-/
                                <xsl:value-of select="$thisTitle"/>/
                                <xsl:value-of select="$page - 1"/>
                            </xsl:attribute>
    &lt;&lt;Prev
                        </a>&#160;
                        <a>
                            <xsl:attribute name="href">
                                <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                <xsl:value-of select="$thisPid"/>/-/
                                <xsl:value-of select="$thisTitle"/>/
                                <xsl:value-of select="$page + 1"/>
                            </xsl:attribute>
    Next>>
                        </a>
                    </xsl:otherwise>
                </xsl:choose>
            </div>
        </xsl:if>
    </xsl:template>
    <xsl:template match="sparql/results">
        <xsl:for-each select="result[position() mod $cellsPerRow = 1 and position()>=$start and position() &lt;=$end]">
            <tr>
                <xsl:apply-templates select=". | following-sibling::result[position() &lt; $cellsPerRow]"/>
            </tr>
        </xsl:for-each>
    </xsl:template>
    <xsl:template match="result">
        <xsl:variable name='OBJECTURI' select="object/@uri"/>
        <xsl:variable name='PID' select="substring-after($OBJECTURI,'/')"/>
        <xsl:variable name="newTitle" >
            <xsl:call-template name="replace-string">
                <xsl:with-param name="text" select="title"/>
                <xsl:with-param name="from" select="'_'"/>
                <xsl:with-param name="to" select="' '"/>
            </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="linkUrl">
            <xsl:choose>
                <xsl:when test="(content='Collection' or content='Community')">
                    <xsl:value-of select="$BASEURL"/>/fedora/repository/
                    <xsl:copy-of select="$PID"/>/-/
                    <xsl:value-of select="title"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$BASEURL"/>/fedora/repository/
                    <xsl:copy-of select="$PID"/>/OBJ/
                    <xsl:value-of select="title"/>.pdf <!-- we know in riri that all OBJ streams are pdf so we can add the extension to tell browsers what it is-->
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <td valign="top" width="25%">
            <a>
                <xsl:attribute name="href">
                    <xsl:value-of select="$linkUrl"/>
                </xsl:attribute>
                <img>
                    <xsl:attribute name="src">
                        <xsl:value-of select="$BASEURL"/>/fedora/repository/
                        <xsl:value-of select="$PID"/>/TN
                    </xsl:attribute>
                    <xsl:attribute name="alt">
                        <xsl:value-of select="$newTitle"/>
                    </xsl:attribute>
                </img>
            </a>
            <br clear="all" />
            <a>
                <xsl:attribute name="href">
                    <xsl:value-of select="$linkUrl"/>
                </xsl:attribute>
                <xsl:value-of select="$newTitle"/>
            </a>
            <xsl:if test="(content!='Collection' and content!='Community')">
                <br />--
                <a>
                    <xsl:attribute name="href">
                        <xsl:value-of select="$BASEURL"/>/fedora/repository/
                        <xsl:copy-of select="$PID"/>/-/
                        <xsl:value-of select="title"/>
                    </xsl:attribute>
    DETAILS
                </a>--
            </xsl:if>
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