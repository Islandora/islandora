<xsl:stylesheet xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" exclude-result-prefixes="mods" version="1.0">
<xsl:output indent="yes" method="html"/>
<xsl:variable name="dictionary" select="document('http://www.loc.gov/standards/mods/modsDictionary.xml')/dictionary"/>

<xsl:template match="/">
<html>
<head>
<style type="text/css">
.modsLabelTop {
}

.modsLabelLevel2 {
  padding-left: 10px;
}

.modsLabelLevel3 {
  padding-left: 20px;
}

.modsLabelLevel4 {
  padding-left: 30px;
}

.modsValueTop {
}

.modsValueLevel2 {
}

.modsValueLevel3 {
}

</style>
</head>
<body>
  <xsl:choose>
    <xsl:when test="mods:modsCollection">
      <xsl:apply-templates select="mods:modsCollection"/>
    </xsl:when>
    <xsl:when test="mods:mods">
      <xsl:apply-templates select="mods:mods"/>
    </xsl:when>
  </xsl:choose>
</body>
</html>
</xsl:template>

<xsl:template match="mods:modsCollection">
  <xsl:apply-templates select="mods:mods"/>
</xsl:template>

<xsl:template match="mods:mods">
  <table class="modsContainer">
  <xsl:apply-templates/>
  </table>
  <!--hr/-->
</xsl:template>

<xsl:template match="*">
  <xsl:choose>
    <xsl:when test="child::*">
      <tr><td colspan="2">
      <span class="modsLabelTop">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>:
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td></tr>
      <xsl:apply-templates mode="level2"/>
    </xsl:when>
    <xsl:otherwise>
      <tr><td>
      <span class="modsLabelTop">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td><td>
      <span class="modsValueTop">
      <xsl:call-template name="formatValue"/>
      </span>
      </td></tr>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="formatValue">
  <xsl:choose>
    <xsl:when test="@type='uri'">
      <a href="{text()}" class="modsLink">
      <xsl:value-of select="text()"/>
      </a>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="text()"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="*" mode="level2">
  <xsl:choose>
    <xsl:when test="child::*">
      <tr><td colspan="2">
      <span class="modsLabelLevel2">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td></tr>
      <xsl:apply-templates mode="level3"/>
    </xsl:when>
    <xsl:otherwise>
      <tr><td>
      <span class="modsLabelLevel2">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td><td>
      <span class="modsValueLevel2">
      <xsl:call-template name="formatValue"/>
      </span>
      </td></tr>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="*" mode="level3">
  <xsl:choose>
    <xsl:when test="child::*">
      <tr><td colspan="2">
      <span class="modsLabelLevel3">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td></tr>
      <xsl:apply-templates mode="level4"/>
    </xsl:when>
    <xsl:otherwise>
      <tr><td>
      <span class="modsLabelLevel3">
      <xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="local-name()"/>
        </xsl:with-param>
      </xsl:call-template>
      <xsl:call-template name="attr"/>
      </span>
      </td><td>
      <span class="modsValueLevel3">
      <xsl:call-template name="formatValue"/>
      </span>
      </td></tr>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="*" mode="level4">
  <tr><td>
  <span class="modsLabelLevel4">
  <xsl:call-template name="longName">
    <xsl:with-param name="name">
      <xsl:value-of select="local-name()"/>
    </xsl:with-param>
  </xsl:call-template>
  <xsl:call-template name="attr"/>
  </span>
  </td><td>
  <span class="modsValueLevel4">
  <xsl:value-of select="text()"/>
  </span>
  </td></tr>
</xsl:template>

<xsl:template name="longName">
  <xsl:param name="name"/>
  <xsl:choose>
    <xsl:when test="$dictionary/entry[@key=$name]">
      <xsl:value-of select="$dictionary/entry[@key=$name]"/>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$name"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="attr">
  <xsl:for-each select="@type|@point">:
    <xsl:call-template name="longName">
      <xsl:with-param name="name">
        <xsl:value-of select="."/>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:for-each>
  <xsl:if test="@authority or @edition">
    <xsl:for-each select="@authority">(<xsl:call-template name="longName">
        <xsl:with-param name="name">
          <xsl:value-of select="."/>
        </xsl:with-param>
      </xsl:call-template>
    </xsl:for-each>
    <xsl:if test="@edition">
      Edition <xsl:value-of select="@edition"/>
    </xsl:if>)
  </xsl:if>
  <xsl:variable name="attrStr">
    <xsl:for-each select="@*[local-name()!='edition' and local-name()!='type' and local-name()!='authority' and local-name()!='point']">
      <xsl:value-of select="local-name()"/>="<xsl:value-of select="."/>",
    </xsl:for-each>
  </xsl:variable>
  <xsl:variable name="nattrStr" select="normalize-space($attrStr)"/>
    <xsl:if test="string-length($nattrStr)">
      (<xsl:value-of select="substring($nattrStr,1,string-length($nattrStr)-1)"/>)
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
