<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:php="http://php.net/xsl" exclude-result-prefixes="php">
    <xsl:variable name="OBJECTSPAGE">
        <xsl:value-of select="$objectsPage"/>
    </xsl:variable>
    <xsl:variable name="ALLOWEDNAMESPACES" select="$allowedPidNameSpaces"/>
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

        <xsl:variable name="SEARCHURL">
		 	<!--<xsl:value-of select="$searchUrl"/>-->
                        mnpl_advanced_search
        </xsl:variable>
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
                <style type="text/css">
 
		span.highlight
		{
		background-color:yellow
		}

		span.searchtitle
		{
		font-weight: bold
		}
                </style>

		Total Hits =
                <strong>
                    <xsl:value-of select="@hitTotal"/>,
                </strong>
		 Number of Hits/page =
                <xsl:value-of select="$HITPAGESIZE"/>
                <br />You may not have sufficient privileges to view any or all of the items found.  The objects you have rights to view will be shown below.
		<!-- Current page = <xsl:value-of select="@hitPageStart"/>-->
                <br/>
                <xsl:if test="$HITTOTAL > $HITPAGESIZE">
                    <div class="item-list">
                        <ul class="pager">
                            <xsl:if test="$HITPAGENO > 1">
                                <li class="pager-previous">
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/mnpl_advanced_search/
                                            <xsl:value-of select="$QUERY"/>/
                                            <xsl:value-of select="$HITPAGESTARTT - $HITPAGESIZE"/>
                                        </xsl:attribute>
                  &lt; Previous
                                    </a>
                                </li>
                            </xsl:if>
                            <xsl:text>  </xsl:text>
                            <xsl:if test="$HITPAGENO &lt; $HITPAGENOLAST">
                                <li class="pager-next">
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/mnpl_advanced_search/
                                            <xsl:value-of select="$QUERY"/>/
                                            <xsl:value-of select="$HITPAGESTARTT+$HITPAGESIZE"/>
                                        </xsl:attribute>
                  Next >
                                    </a>
                                </li>
                            </xsl:if>
                        </ul>
                    </div>
                </xsl:if>
		<!--<form action="/drupal-5.1/?q=search/fedora_collections"  method="post" id="search-form" class="search-form">-->
		
                <xsl:apply-templates select="objects"/>
                <xsl:if test="$HITTOTAL > $HITPAGESIZE">
                    <div class="item-list">
                        <ul class="pager">
                            <xsl:if test="$HITPAGENO > 1">
                                <li class="pager-previous">
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/mnpl_advanced_search/
                                            <xsl:value-of select="$QUERY"/>/
                                            <xsl:value-of select="$HITPAGESTARTT - $HITPAGESIZE"/>
                                        </xsl:attribute>
                  &lt; Previous
                                    </a>
                                </li>
                            </xsl:if>
                            <xsl:text>  </xsl:text>
                            <xsl:if test="$HITPAGENO &lt; $HITPAGENOLAST">
                                <li class="pager-next">
                                    <a>
                                        <xsl:attribute name="href">
                                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/mnpl_advanced_search/
                                            <xsl:value-of select="$QUERY"/>/
                                            <xsl:value-of select="$HITPAGESTARTT+$HITPAGESIZE"/>
                                        </xsl:attribute>
                  Next >
                                    </a>
                                </li>
                            </xsl:if>
                        </ul>
                    </div>
                </xsl:if>
            </xsl:when>
            <xsl:otherwise>
                <div class="box">

                    <h2>Your search yielded no results</h2>

                    <div class="content">
                        <ul>
                            <li>Check if your spelling is correct.</li>

                            <li>Remove quotes around phrases to match each word individually:
                                <em>"blue smurf"</em> will match less than
                                <em>blue smurf</em>.
                            </li>
                            <li>Consider loosening your query with
                                <em>OR</em>:
                                <em>blue smurf</em> will match less than
                                <em>blue OR smurf</em>.
                            </li>
                        </ul>
                    </div>

                </div>

            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template match="objects">




        <table>
            <div class="search-results">
                <xsl:for-each select="object">
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


                    <xsl:call-template name="splitBySpace">
                        <xsl:with-param name="PIDVALUE" select="$PIDVALUE"></xsl:with-param>
                        <xsl:with-param name="str" select="$ALLOWEDNAMESPACES"/>
                    </xsl:call-template>


                </xsl:for-each>
            </div>
        </table>

    </xsl:template>
    <xsl:template name="splitBySpace">
        <xsl:param name="str"/>
        <xsl:param name="PIDVALUE"/>
		
		
        <xsl:choose>
            <xsl:when test="contains($str,' ')">
				<!--'DO SOMETHING WITH THE VALUE IN
				{substring-before($str,' ')}-->
                <xsl:variable name="testString" select="substring-before($str,' ')"/>
				
                <xsl:if test="starts-with(php:function('strtolower',$PIDVALUE),php:function('strtolower',$testString))">
				 	
                    <xsl:call-template name="showResult">
						
                        <xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>
						
                    </xsl:call-template>
				 	
                </xsl:if>
				
				<!--<xsl:value-of	select="substring-before($str,' ')"/>-->
                <xsl:call-template name="splitBySpace">
                    <xsl:with-param name="str"
					select="substring-after($str,' ')"/>
                    <xsl:with-param name="PIDVALUE"
					select="$PIDVALUE"/>
					
					
                </xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <!-- <xsl:if test="substring-before($str, ':') = substring-before($PIDVALUE, ':')">

                     <xsl:call-template name="showResult">
                         <xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>
                     </xsl:call-template>

                 </xsl:if>-->
            </xsl:otherwise>
        </xsl:choose>
    </xsl:template>

    <xsl:template name="showResult">
        <xsl:param name="PIDVALUE"/>
        <xsl:variable name="DCTITLE">
            <xsl:value-of select="field[@name='dc.title']/node()"/>
        </xsl:variable>

        <xsl:variable name="CLEANTITLE">
            <xsl:value-of select="php:functionString('fedora_repository_urlencode_string', $DCTITLE)"/>
        </xsl:variable>
        <tr>
            <td valign="top">
                <a>
                    <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                        <xsl:copy-of select="$PIDVALUE"/>/-/
                        <xsl:value-of select="$CLEANTITLE"/>
                    </xsl:attribute>
			<!--<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/><![CDATA[&pid=]]><xsl:value-of select="$PIDVALUE"/><![CDATA[&collection=object]]>

				</xsl:attribute>-->

      <!-- an example hitting j2k for the thumbnail  <img>
              <xsl:attribute name="alt">Thumbnail created for djatoka from object <xsl:copy-of select="$PIDVALUE"/></xsl:attribute>
              <xsl:attribute name="src">
      http://islandlives.ca:8080/adore-djatoka/resolver?url_ver=Z39.88-2004&amp;rft_id=http://islandlives.ca/fedora/repository/<xsl:copy-of select="$PIDVALUE"/>/JPG/jpg.jpg&amp;svc_id=info:lanl-repo/svc/getRegion&amp;svc_val_fmt=info:ofi/fmt:kev:mtx:jpeg&amp;svc.format=image/jpeg&amp;svc.level=0&amp;svc.rotate=0&amp;svc.region=0,0,100,100</xsl:attribute>
            </img>-->

                    <img>
                        <xsl:attribute name="src">
                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                            <xsl:copy-of select="$PIDVALUE"/>/TN
                        </xsl:attribute>
                    </img>
                </a>

            </td  >

            <td width="80%" valign="top">
                <table  valign="top">


                    <tr valign="top">
                        <td  valign="top" class="search-results" colspan="2">


                            <xsl:value-of select="@no"/>
                            <xsl:value-of select="'. '"/>




                            <a>
							<!--<xsl:attribute name="href"><![CDATA[http://localhost/drupal-5.1/?q=node/7&pid=]]><xsl:value-of select="$PIDVALUE"/><![CDATA[&collection=object]]>-->
							<!--<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/><![CDATA[&pid=]]><xsl:value-of select="$PIDVALUE"/><![CDATA[&collection=object]]>-->
                                <xsl:attribute name="href">
                                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                                    <xsl:value-of select="$PIDVALUE"/>/-/
                                    <xsl:value-of select="$CLEANTITLE"/>

                                </xsl:attribute>

                                <xsl:value-of select="$PIDVALUE"/>
                                <br />
                            </a>
                            <span class="searchtitle">
                                <span >
								Score:(
                                    <xsl:value-of select="@score"/>)
                                </span>
                                <br />
                                <a>
								<!--<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/><![CDATA[&pid=]]><xsl:value-of select="$PIDVALUE"/><![CDATA[&collection=object]]>-->
                                
                                    <xsl:attribute name="href">
                                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                                        <xsl:value-of select="$PIDVALUE"/>/-/
                                        <xsl:value-of select="$CLEANTITLE"/>

                                    </xsl:attribute>
                                    <xsl:value-of select="field[@name='dc.title']/node()" disable-output-escaping="yes"/>
                                </a>
                            </span>


                        </td>


                    </tr>



                    <xsl:for-each select="field[@snippet='yes']">
                        <xsl:choose>
                            <xsl:when test="(@name='fgs.DS.first.text')">

                                <tr>
                                    <td  valign="top">
                                        <span class="searchtitle">
										Text Stream<!--<xsl:value-of select="@name"/>-->
                                        </span>
                                    </td>
                                    <td>
                                        <span class="text">
                                            <xsl:copy-of select="node()"/>
                                        </span>
                                    </td>
                                </tr>
                            </xsl:when>
                            <xsl:when test="(@name='dc.title')">

                            </xsl:when>
                            <xsl:otherwise>
                                <tr>
                                    <td  valign="top">
                                        <span class="searchtitle">
                                            <xsl:value-of select="@name"/>
                                        </span>
                                    </td>
                                    <td>
                                        <span class="text">
                                            <xsl:copy-of select="node()"/>
                                        </span>
                                    </td>
                                </tr>
                            </xsl:otherwise>
                        </xsl:choose>
                    </xsl:for-each>

                </table>

            </td>
        </tr>
    </xsl:template>

</xsl:stylesheet>
