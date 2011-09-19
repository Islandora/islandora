<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0" xmlns:xs="http://www.w3.org/2001/XMLSchema"
    targetNamespace="http://rs.tdwg.org/dwc/xsd/simpledarwincore/"
    xmlns:dc="http://purl.org/dc/terms/"
    xmlns:dwr="http://rs.tdwg.org/dwc/xsd/simpledarwincore/"
    xmlns:dwc="http://rs.tdwg.org/dwc/terms/"> 
    <xs:import namespace="http://rs.tdwg.org/dwc/terms/" schemaLocation="tdwg_dwcterms.xsd"/> 
    <xs:import namespace="http://purl.org/dc/terms/" schemaLocation="dublin_core.xsd"/>
 <xsl:output method="xml" omit-xml-declaration="yes"/>
 <!-- SPECIMEN XSLT -->
 <xsl:template match="/">


<table>
	<tr cellpadding="10"><th><b>Specimen</b></th></tr>
	<tr class="odd"><td width="200">Scientific Name</td><td><xsl:value-of select="//dwc:scientificName"/></td></tr>
	<tr class="even"><td width="200">Family</td><td><xsl:value-of select="//dwc:family"/></td></tr>
	<tr class="odd"><td width="200">Genus</td><td><xsl:value-of select="//dwc:genus"/></td></tr>
	<tr class="even"><td width="200">Common Name</td><td><xsl:value-of select="//dwc:vernacularName"/></td></tr>

	<tr cellpadding="10"><th><b>Location</b></th></tr>
	<tr class="odd"><td width="200">Continent</td><td><xsl:value-of select="//dwc:continent"/></td></tr>
	<tr class="even"><td width="200">Country</td><td><xsl:value-of select="//dwc:country"/></td></tr>
	<tr class="odd"><td width="200">Country Code</td><td><xsl:value-of select="//dwc:countryCode"/></td></tr>
	<tr class="even"><td width="200">State/Province</td><td><xsl:value-of select="//dwc:stateProvince"/></td></tr>
	<tr class="odd"><td width="200">County</td><td><xsl:value-of select="//dwc:county"/></td></tr>
	<tr class="even"><td width="200">Locality</td><td><xsl:value-of select="//dwc:locality"/></td></tr>
	<tr class="odd"><td width="200">Habitat</td><td><xsl:value-of select="//dwc:habitat"/></td></tr>

	<tr cellpadding="10"><th><b>Record</b></th></tr>
	<tr class="odd"><td width="200">Record Type</td><td><xsl:value-of select="//dc:type"/></td></tr>
	<tr class="even"><td width="200">Language- </td><td><xsl:value-of select="//dc:language"/></td></tr>
	<tr class="odd"><td width="100">Record Basis</td><td><xsl:value-of select="//dwc:basisOfRecord"/></td></tr>
	<tr class="even"><td width="200">Occurence Remarks</td><td><xsl:value-of select="//dwc:occurenceRemarks"/></td></tr>
	<tr class="odd"><td width="200">Occurence ID</td><td><xsl:value-of select="//dwc:occurenceID"/></td></tr>
	<tr class="even"><td width="200">Institution Code</td><td><xsl:value-of select="//dwc:institutionCode"/></td></tr>
	<tr class="odd"><td width="200">Collection Code</td><td><xsl:value-of select="//dwc:collectionCode"/></td></tr>
	<tr class="even"><td width="200">Catalog Number</td><td><xsl:value-of select="//dwc:catalogNumber"/></td></tr>
	<tr class="odd"><td width="200">Recorded By</td><td><xsl:value-of select="//dwc:recordedBy"/></td></tr>
	<tr class="even"><td width="200">Event Date200</td><td><xsl:value-of select="//dwc:eventDate"/></td></tr>
</table>	
 </xsl:template>
</xsl:stylesheet>
