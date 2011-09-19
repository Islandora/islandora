<?xml version="1.0" encoding="UTF-8"?> 
<xsl:stylesheet version="1.0"
		xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns:zs="http://www.loc.gov/zing/srw/"
		xmlns:foxml="info:fedora/fedora-system:def/foxml#"
		xmlns:dc="http://purl.org/dc/elements/1.1/"
        xmlns:php="http://php.net/xsl" exclude-result-prefixes="php">
		
<!-- This xslt stylesheet generates the resultPage
     from a Lucene browseIndex.
-->
	
    <xsl:output method="xml" indent="yes" encoding="UTF-8"/>
    <xsl:param name="STARTTERM" select="query"/>
    <xsl:param name="TERMPAGESIZE" select="10"/>
    <xsl:param name="RESULTPAGEXSLT" select="resultPageXslt"/>
    <xsl:param name="DATETIME" select="none"/>
    <xsl:variable name="OBJECTSPAGE" select="$objectsPage"/>
     <xsl:variable name="DISPLAYNAME" select="$displayName"/>
	
    <xsl:variable name="INDEXNAME" select="lucenebrowseindex/@indexName"/>
    <xsl:variable name="FIELDNAME" select="lucenebrowseindex/@fieldName"/>
    <xsl:variable name="TERMTOTAL" select="lucenebrowseindex/@termTotal"/>
    <xsl:variable name="PAGELASTTERM" select="lucenebrowseindex/terms/term[position()=last()]/text()"/>
    <xsl:template match="lucenebrowseindex">
      
        
        <resultPage dateTime="{$DATETIME}"
	 				indexName="{$INDEXNAME}">
            <browseIndex 	startTerm="{$STARTTERM}"
	 						fieldName="{$FIELDNAME}"
	 						termPageSize="{$TERMPAGESIZE}"
	 						resultPageXslt="{$RESULTPAGEXSLT}"
	 						termTotal="{$TERMTOTAL}">
  <h3><xsl:value-of select="$DISPLAYNAME"/></h3>
  <!--start pager Div-->
<!--Start letters -->
<div class="item-list">
     <ul class="pager">
       <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/A/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> A </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/B/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> B </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/C/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> C </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/D/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> D </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/E/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> E </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/F/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> F </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/G/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> G </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/H/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> H </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/I/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> I </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/J/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> J </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/K/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> K </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/L/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> L </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/M/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> M </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/N/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> N </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/O/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> O </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/P/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> P </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Q/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Q </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/R/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> R </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/S/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> S </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/T/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> T </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/U/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> U </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/V/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> V </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/W/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> W </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/X/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> X </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Y/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Y </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Z/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Z </span>
                </a>
                </li>
<!--End letters-->

             <li class="pager-next">
                <a>
                    <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/<xsl:value-of select="$PAGELASTTERM"/>/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> >>
                </a>
                </li>
                </ul>
                </div>
                <!--End pager Div-->
                <br /><br />
                <xsl:apply-templates select="terms"/>
                <br />
  <!--start pager Div-->
<!--Start letters -->
<div class="item-list">
     <ul class="pager">
       <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/A/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> A </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/B/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> B </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/C/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> C </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/D/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> D </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/E/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> E </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/F/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> F </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/G/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> G </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/I/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> H </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/I/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> I </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/J/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> J </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/K/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> K </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/L/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> L </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/M/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> M </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/N/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> N </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/O/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> O </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/P/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> P </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Q/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Q </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/R/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> R </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/S/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> S </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/T/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> T </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/U/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> U </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/V/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> V </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/W/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> W </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/X/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> X </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Y/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Y </span>
                </a>
                </li>
                <li class="pager-item">
                <a> <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/Z/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> <span class="letter"> Z </span>
                </a>
                </li>
<!--End letters-->

             <li class="pager-next">
                <a>
                    <xsl:attribute name="href">
                        <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/list_terms/<xsl:value-of select='$FIELDNAME'/>/<xsl:value-of select="$PAGELASTTERM"/>/<xsl:value-of select="$DISPLAYNAME"/>
                    </xsl:attribute> >>
                </a>
                </li>
                </ul>
                </div>
                <!--End pager Div-->
				<!--<xsl:copy-of select="fields"/>-->
				<!--<xsl:copy-of select="terms"/>-->
            </browseIndex>
        </resultPage>
    </xsl:template>

    <xsl:template match="terms">
        <xsl:for-each select="term">
            <a>
                <xsl:attribute name="href">
                   <!-- <xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/mnpl_advanced_search/<xsl:value-of select='$FIELDNAME'/>:"<xsl:value-of select="."/>" AND dc.type:collection
                   the commented out url is for general use the one below is for islandlives book view-->
                   <xsl:variable name="SHORTFIELDNAME" select="substring-before($FIELDNAME,'TERM')"/>    
                   <xsl:copy-of select="$OBJECTSPAGE"/>fedora/ilives_book_search/<xsl:value-of select='$SHORTFIELDNAME'/>:"<xsl:value-of select="."/>" AND dc.type:collection
                </xsl:attribute>
                <xsl:value-of select="."/>
            </a>
            <!-- appears in
            <xsl:text> </xsl:text>
            <xsl:value-of select="@fieldtermhittotal"/> documents -->
            <br />
        </xsl:for-each>
    
    </xsl:template>
	
</xsl:stylesheet>	




				




