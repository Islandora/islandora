<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"  >
    <xsl:variable name="OBJECTSPAGE">
        <xsl:value-of select="$objectsPage"/>
    </xsl:variable>
  <!--<xsl:variable name="ALLOWEDNAMESPACES" select="$allowedPidNameSpaces"/>-->
    <xsl:variable name="cellsPerRow" select="6"/>
    <xsl:variable name="USER" select="$userID"/>
  
    <!--<xsl:variable name="count" select="count(//objects)"/>-->
<!--<xsl:variable name="PATHTOMAKEIMAGE">
		 	<xsl:value-of select="$pathToMakeImage"/>
		</xsl:variable>-->

    <xsl:template match="gfindObjects">
  
        <xsl:variable name="INDEXNAME" select="@indexName"/>

        <xsl:variable name="PREQUERY" select="substring-before(@query,':')"/>
		<!--<xsl:variable name="QUERY" select="substring-after(@query,':')"/>-->
        <xsl:variable name="QUERY" select="@query"/>
        <xsl:variable name="HITPAGESTART" select="@hitPageStart"/>
        <xsl:variable name="HITPAGESIZE" select="@hitPageSize"/>
        <xsl:variable name="HITTOTAL" select="@hitTotal"/>

   
        <xsl:variable name="TOKEN">
            <xsl:value-of select="$searchToken"/>
      
        </xsl:variable>
        <xsl:variable name="HITPAGEEND">
            <xsl:choose>
                <xsl:when test="$HITPAGESTART + $HITPAGESIZE - 1 > $HITTOTAL">
                    <xsl:value-of select="$HITTOTAL"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="$HITPAGESTART + $HITPAGESIZE - 1"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>

        <xsl:variable name="HITPAGENO" select="ceiling($HITPAGESTART div $HITPAGESIZE)"/>
        <xsl:variable name="HITPAGENOLAST" select="ceiling($HITTOTAL div $HITPAGESIZE)"/>
        <xsl:variable name="HITPAGESTARTT" select="(($HITPAGENO - 1) * $HITPAGESIZE + 1)"/>
        <xsl:choose>
            <xsl:when test="$HITTOTAL > 0">
     

		Total Hits =
                <strong>
                    <xsl:value-of select="@hitTotal"/>,
                </strong>
		<!-- Number of Hits/page = <xsl:value-of select="@hitPageSize"/>-->
                <br />We have repeated your search within this book and found results on the following pages.
		<!-- Current page = <xsl:value-of select="@hitPageStart"/>-->
                <br/>
     
                <xsl:apply-templates select="objects">
                    <xsl:with-param name="end" select="$HITTOTAL"/>
                </xsl:apply-templates>
            </xsl:when>
      <!--<xsl:otherwise>
        <div class="box">
          <h2>We have repeated your search within this book and have not found any results.</h2>
        </div>
      </xsl:otherwise>-->
        </xsl:choose>
    </xsl:template>

    <xsl:template match="objects">
        <xsl:param name="end"/>
        <div class="search-results">
            <table>
                <xsl:for-each select="object[position() mod $cellsPerRow = 1 and position()>=0 and position() &lt;=$end]">
                    <xsl:sort select="field[@name='PID']/text()" order='ascending'/>
                    <tr>
                        <xsl:apply-templates select=". | following-sibling::object[position() &lt; $cellsPerRow]">
                            <xsl:sort select="field[@name='PID']/text()" order='ascending'/>
                        </xsl:apply-templates>
                    </tr>
                </xsl:for-each>
            </table>
        </div>
    </xsl:template>

    <xsl:template match="object">
        <xsl:variable name="PIDVALUE">
            <xsl:choose>
                <xsl:when test="@PID">
                    <xsl:value-of select="@PID"/>
                </xsl:when>
                <xsl:when test="field[@name='PID' and @snippet='yes']">
                    <xsl:value-of select="field[@name='PID']/span/text()"/>
                </xsl:when>
                <xsl:otherwise>
                    <xsl:value-of select="field[@name='PID']/text()"/>
                </xsl:otherwise>
            </xsl:choose>
        </xsl:variable>
        <xsl:call-template name="showResult">
            <xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>
           
        </xsl:call-template>
    </xsl:template>
  
    <xsl:template name="showResult">
        <xsl:param name="PIDVALUE"/>
        <xsl:variable name="DCTITLE">
            <xsl:value-of select="field[@name='dc.title']/node()"/>
        </xsl:variable>

   <!-- <xsl:variable name="CLEANTITLE">
      <xsl:value-of select="php:functionString('fedora_repository_urlencode_string', $DCTITLE)"/>
    </xsl:variable>-->
        <xsl:variable name="recordNo">
            <xsl:value-of select="position()"/>
        </xsl:variable>
        <td valign="top" width="16%">
            <a>
                <xsl:attribute name="href">
                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/ilives_book_viewer/
                    <xsl:copy-of select="$PIDVALUE"/>
                </xsl:attribute>
                <img>
                    <xsl:attribute name="alt">Thumbnail
                        <xsl:copy-of select="$PIDVALUE"/>
                    </xsl:attribute>
                    <xsl:attribute name="src">
      <!--http://islandlives.ca:8080/adore-djatoka/resolver?url_ver=Z39.88-2004&amp;rft_id=http://islandlives.ca/fedora/repository/<xsl:copy-of select="$PIDVALUE"/>/JP2/jp2.jp2&amp;svc_id=info:lanl-repo/svc/getRegion&amp;svc_val_fmt=info:ofi/fmt:kev:mtx:jpeg2000&amp;svc.format=image/jpeg&amp;svc.level=0&amp;svc.rotate=0&amp;svc.region=0,0,100,100</xsl:attribute>
            -->
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                        <xsl:copy-of select="$PIDVALUE"/>/TN
                    </xsl:attribute>
                </img>
            </a>
            <br />
            <a>
                <xsl:attribute name="href">
                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/ilives_book_viewer/
                    <xsl:value-of select="$PIDVALUE"/>
                </xsl:attribute>
                <xsl:value-of select="substring-before($DCTITLE, '-')"/>
                <br />
            </a>
        </td  >
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

</xsl:stylesheet>
