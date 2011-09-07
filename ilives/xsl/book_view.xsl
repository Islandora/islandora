<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:mods="http://www.loc.gov/mods/v3">
    <xsl:variable name="OBJECTSPAGE">
        <xsl:value-of select="$objectsPage"/>
    </xsl:variable>
    <xsl:variable name="PID">
        <xsl:value-of select="$pid"/>
    </xsl:variable>
    <xsl:variable name="RECORDID">
        <xsl:value-of select='substring-after($PID,":")'/>
    </xsl:variable>
    <xsl:variable name="USER" select="$userID"/>
    <xsl:variable name="INGESTED" select="$ingested"/>

    <xsl:template match="/">

        <h1 >
            <xsl:value-of select="//mods:titleInfo/mods:nonSort"/>
            <xsl:text> </xsl:text>
            <xsl:value-of select="//mods:titleInfo/mods:title"/>
            <xsl:for-each select="//mods:titleInfo/mods:subTitle">
                               :
                <xsl:value-of select="."/>
            </xsl:for-each>
        </h1>
        <xsl:for-each select="//mods:titleInfo[@type='alternative']/mods.title">
            <h3>
                               <!-- <xsl:value-of select="substring-before(text(),'/')"/>-->
                <xsl:value-of select="."/>
                <xsl:text> </xsl:text>
            </h3>
        </xsl:for-each>
        <table cellpadding="3" cellspacing="2" valign="top" >
            <tr>
                <td valign="top">
                    <img>
                        <xsl:attribute name="src">
                            <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                            <xsl:copy-of select="$PID"/>/TN
                        </xsl:attribute>
                    </img>



                </td>
                <td >
                    <div style="align:left">
                        <table cellpadding="3" cellspacing="2" width="90%" >
                            <tr>
                                <td align="right" valign="top">
                                    <b>By Statement: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:note[@type='statement of responsibility']"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Place of Publication: </b>
                                </td >
                                <td valign="top">
                                    <xsl:value-of select="//mods:placeTerm[@type='text']"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Publisher: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:publisher"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Date: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:dateIssued"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Language: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:languageTerm"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Pagination: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:extent"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>ISBN 10: </b>
                                </td>
                                <td valign="top">
                                    <xsl:value-of select="//mods:identifier[@type='isbn']"/>
                                </td>
                            </tr>
                            <tr>
                                <td align="right" valign="top">
                                    <b>Subjects: </b>
                                </td>
                                <td valign="top">
                                    <xsl:for-each select="//mods:subject">
                                        <xsl:for-each select="*">
                                            <a>
                                                <xsl:attribute name="href">
                                                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/ilives_book_search/mods.subject:"
                                                    <xsl:value-of select="normalize-space(.)"/>"%20AND%20dc.type:collection%20AND%20dc.type:ingested
                                                </xsl:attribute>
                                    
                                                <xsl:value-of select="."/>
                                            </a>
                                            <xsl:text> </xsl:text>
                                        </xsl:for-each>
                                        <br />
                                    </xsl:for-each>
                                </td>
                            </tr>
                        </table>
                    </div>
                </td>
                <td valign="top">
                    <xsl:if test="$INGESTED = 'true'">
                        <div>
                            <a>
                                <xsl:attribute name="href">
                                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/ilives_book_viewer/
                                    <xsl:value-of select="$PID"/>
                                </xsl:attribute>Read
                            </a>
                        </div>
                    </xsl:if>
                    <div>
                        <a>
                            <xsl:attribute name="href">http://islandpines.roblib.upei.ca/opac/en-CA/skin/roblib/xml/rdetail.xml?r=
                                <xsl:value-of select="$RECORDID"/>
                            </xsl:attribute>
Checkout
                        </a>
                    </div>
                    <xsl:if test="$INGESTED = 'true'">
                        <div>
                            <a>
                                <xsl:attribute name="href">
                                    <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/
                                    <xsl:value-of select="$PID"/>/PDF/
                                    <xsl:value-of select="$PID"/>.pdf
                                </xsl:attribute>Download
                            </a>
                        </div>
                    </xsl:if>
                    <xsl:if test="($USER > 0) and ($INGESTED = 'true')">
                        <div>
                            <a>
                                <xsl:attribute name="href">
                                    <xsl:copy-of select="$OBJECTSPAGE"/>teieditor/
                                    <xsl:value-of select="$PID"/>
                                </xsl:attribute>Edit
                            </a>
                        </div>
                    </xsl:if>
                </td>
            </tr>
        </table>

    </xsl:template>
</xsl:stylesheet>