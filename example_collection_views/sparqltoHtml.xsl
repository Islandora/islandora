<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
 <!-- DEFAULT XSLT FOR COLLECTIONS THAT DO NOT DEFINE THEIR OWN-->
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
    <xsl:variable name='columns' select="4"/>
    <xsl:variable name="count" select="count(sparql/results/result)"/>
    <xsl:template match="/">
        <xsl:if test="$count>0">
  
            <table width='100%'>
                <tr>
                    <td colspan="2">
                        <div align="center">
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
                    </td>
                </tr>
 <!--select="//guestbook/entry[position()>=$start and $end>=position()]">-->
                <xsl:for-each select="/sparql/results/result[position()>=$start and position() &lt;=$end]">
                    <xsl:variable name='OBJECTURI' select="object/@uri"/>
                    <xsl:variable name='PID' select="substring-after($OBJECTURI,'/')"/>
                    <tr>
                        <td>
                            <img>
                                <xsl:attribute name="src">
                                    <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                    <xsl:value-of select="$PID"/>/TN
                                </xsl:attribute>
                            </img>
                        </td>
                        <td>
                            <a>
                                <xsl:attribute name="href">
                                    <xsl:value-of select="$BASEURL"/>/fedora/repository/
                                    <xsl:copy-of select="$PID"/>/-/
                                    <xsl:value-of select="title"/>
                                </xsl:attribute>
                                <xsl:value-of select="title"/>
                            </a>
                        </td>
                    </tr>
                </xsl:for-each>
            </table>
            <div align="center">
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
</xsl:stylesheet>