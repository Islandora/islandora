<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:critters="http://vre.upei.ca/critters/">
    <xsl:output method="xml" omit-xml-declaration="yes"/>
 <!-- CRITTER XSLT -->
    <xsl:template match="/">

        <ul>
            <li>
                <b>Date Collected:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:date_collected"/>
            </li>
            <li>
                <b>Type:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:type"/>
            </li>
            <li>
                <b>Phylum:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:phylum"/>
            </li>
            <li>
                <b>SubPhylum:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:SubPhylum"/>
            </li>
            <li>
                <b>Class:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:class"/>
            </li>
            <li>
                <b>Order:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:order"/>
            </li>
            <li>
                <b>Family:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:family"/>
            </li>
            <li>
                <b>Genus:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:genus"/>
            </li>
            <li>
                <b>Species:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:taxonomy/critters:species"/>
            </li>

        </ul>

        <h4>Collection Location</h4>
        <ul>
            <li>
                <b>Site Name:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:sitename"/>
            </li>
            <li>
                <b>Country:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:country"/>
            </li>
            <li>
                <b>Region:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:region"/>
            </li>
            <li>
                <b>Latitude:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:latitude"/>
            </li>
            <li>
                <b>Longitude:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:longitude"/>
            </li>
            <li>
                <b>Depth:</b>
                <xsl:text> </xsl:text>
                <xsl:value-of select="/critters:sample/critters:site/critters:depth"/>
                <xsl:text> </xsl:text>Feet
            </li>
        </ul>
        <h4>Description</h4>
        <div>
            <xsl:value-of select="/critters:sample/critters:description"/>
        </div>

    </xsl:template>
</xsl:stylesheet>