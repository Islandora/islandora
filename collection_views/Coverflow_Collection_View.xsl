
<xsl:stylesheet exclude-result-prefixes="php" version="1.0" xmlns:php="http://php.net/xsl"
  xmlns:s="http://www.w3.org/2001/sw/DataAccess/rf1/result" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:template match="/">
        <xsl:variable name="BASEURL">
            <xsl:value-of select="$baseUrl"/>
        </xsl:variable>
        <xsl:variable name="PATH">
            <xsl:value-of select="$path"/>
        </xsl:variable>
        <xsl:variable name="collTitle" select="/s:sparql/s:results/s:result/s:collTitle"/>
        <xsl:variable name="collDesc" select="/s:sparql/s:results/s:result/s:collDesc"/>
        <script src="http://yui.yahooapis.com/2.7.0/build/yahoo-dom-event/yahoo-dom-event.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script src="http://yui.yahooapis.com/2.7.0/build/animation/animation-min.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script src="http://yui.yahooapis.com/2.7.0/build/element/element-min.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script src="http://yui.yahooapis.com/2.7.0/build/container/container_core-min.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script src="http://yui.yahooapis.com/2.7.0/build/menu/menu-min.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script src="http://yui.yahooapis.com/2.7.0/build/button/button-min.js" type="text/javascript">
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script type="text/javascript">
            <xsl:attribute name="src">
                <xsl:value-of select="$PATH"/>
                <xsl:text>/collection_views/yui_coverflow/js/CoverFlow.js</xsl:text>
            </xsl:attribute>
            <xsl:comment>Comment added so script is recognised</xsl:comment>
        </script>
        <script type="text/javascript">
            <xsl:text>
                
               // YAHOO.util.Event.onDOMReady(function(){
                //$(document).ready(function(){
                window.onload = function(){
                var images = [
            </xsl:text>
            <xsl:for-each select="/s:sparql/s:results/s:result">
                <xsl:variable name="OBJECTURI" select="s:object/@uri"/>
                <xsl:variable name="pid" select="substring-after($OBJECTURI,'/')"/>
                <xsl:text>{src: '</xsl:text>
                <xsl:value-of select="$BASEURL"/>
                <xsl:text>/fedora/repository/</xsl:text>
                <xsl:value-of select="$pid"/>
                <xsl:text>/TN', label: '</xsl:text>
                <xsl:value-of select="s:memberTitle"/>
                <xsl:text>', onclick: function(){alert('image1');}},
                </xsl:text>
            </xsl:for-each>
            <xsl:text>
                ];
                
                var myCoverFlow = new YAHOO.ext.CoverFlow('coverFlowTest', {height: 200, width: 600, images: images});
                
                function moveLeft(e, coverFlow){
                coverFlow.selectNext();
                }
                function moveRight(e, coverFlow){
                coverFlow.selectPrevious();
                }
                var myMoveLeftBtn = new YAHOO.widget.Button('moveLeftButton', {onclick: {fn: moveLeft, obj: myCoverFlow}});
                var myMoveRightBtn = new YAHOO.widget.Button('moveRightButton', {onclick: {fn: moveRight, obj: myCoverFlow}});
                
                };
            </xsl:text>
        </script>
        <div class="title">Testing YUI's CoverFlow version 0.1 (beta)</div>
        <div id="coverFlowTest"/>
        <input id="moveLeftButton" type="button" value="Select Next"/>
        <input id="moveRightButton" type="button" value="Select Previous"/>
        <br/>
        <br/>
    </xsl:template>
</xsl:stylesheet>
