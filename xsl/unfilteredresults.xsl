<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:php="http://php.net/xsl" exclude-result-prefixes="php">
    <xsl:variable name="OBJECTSPAGE">
        <xsl:value-of select="$objectsPage"/>
    </xsl:variable>

    <xsl:template match="gfindObjects">
        <xsl:variable name="INDEXNAME" select="@indexName"/>

        <xsl:variable name="PREQUERY" select="substring-before(@query,':')"/>
        <xsl:variable name="QUERY" select="substring-after(@query,':')"/>
        <xsl:variable name="HITPAGESTART" select="@hitPageStart"/>
        <xsl:variable name="HITPAGESIZE" select="@hitPageSize"/>
        <xsl:variable name="HITTOTAL" select="@hitTotal"/>

        <xsl:variable name="SEARCHURL">
            <xsl:value-of select="$searchUrl"/>
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

		Total Hits =
                <strong>
                    <xsl:value-of select="@hitTotal"/>,
                </strong>
		 Number of Hits/page =
                <xsl:value-of select="@hitPageSize"/>

		<!-- Current page = <xsl:value-of select="@hitPageStart"/>-->
                <br/>
		<!--<form action="/drupal-5.1/?q=search/fedora_collections"  method="post" id="search-form" class="search-form">-->
                <form action="{$SEARCHURL}"  method="post" id="search-form" class="search-form">


                    <input type="hidden" maxlength="255" name="keys" id="edit-keys"  size="40" value="{$HITPAGESTARTT+$HITPAGESIZE}:{$QUERY}" class="form-text" />
		<!--<xsl:if test="$PREQUERY = fgs.DS.first.text" >-->
                    <input type="hidden" name="type" id="edit-type" value="{$PREQUERY}" />
		<!--</xsl:if>-->
                    <xsl:if test="$HITPAGENO > 1">
                        <input type="button" name="back" onClick="javascript:history.go(-1)" id="back-button" value="Previous"  class="form-submit" />
                    </xsl:if>
                    <xsl:if test="$HITPAGENO &lt; $HITPAGENOLAST">
                        <input type="submit" name="op" id="edit-submit" value="Next"  class="form-submit" />
                    </xsl:if>

                    <input type="hidden" name="form_token" id="edit-search-form-form-token" value="{$TOKEN}" />
                    <input type="hidden" name="form_id" id="edit-search-form" value="search_form"  />

                </form>
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
                <xsl:apply-templates select="objects"/>
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


                    <xsl:call-template name="showResult">
                        <xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>
                    </xsl:call-template>

                </xsl:for-each>
            </div>
        </table>

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
