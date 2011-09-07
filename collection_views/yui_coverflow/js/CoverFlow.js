/**
 * @author elmasse (Maximiliano Fierro)
 * @version 0.1 beta
 *
 * @usage:
 * <code> 
 * 	var images = [
 * 				  {src: 'images/ardillitaMac.jpg'},
 *	              {src: 'http://farm2.static.flickr.com/1380/1426855399_b4b8eccbdb.jpg?v=0'},
 *	              {src: 'http://farm1.static.flickr.com/69/213130158_0d1aa23576_d.jpg'}
 *	              ];
 *	var myCoverFlow = new YAHOO.ext.CoverFlow('coverFlowTest', {height: 200, width: 800, images: images, bgColor: '#C0C0C0'});
 *	</code>
 *
 */


YAHOO.namespace("ext");

//(function(){

/**
	 * @class CoverFlow 
	 * @namespace YAHOO.ext
	 * @constructor
	 * @param el {String|HTMLElement} Reference to element where CoverFlow will be rendered.
	 * @param config {Object} configuration object
	 *        config.height {Number} Element height. Optional. Default: CoverFlow.DEFAULT_HEIGHT.
	 *        config.width  {Number} Element width. Optional. Default: CoverFlow.DEFAULT_WIDTH.
	 *        config.images {Array} Array of Images. [{src:}]
	 *        config.bgColor {String} Background color. Could be in the form #00000 or black. Optional. Default: CoverFlow.DEFAULT_BG_COLOR. 
	 *        
	 */
YAHOO.ext.CoverFlow = function(el, userConfig){
    if(el)
        this.init(el, userConfig || {});
};
	
//shortcuts
var CoverFlow = YAHOO.ext.CoverFlow; 
var Dom = YAHOO.util.Dom;

	
/**
	 * Defaults
	 */
CoverFlow.DEFAULT_HEIGHT = 300;
CoverFlow.DEFAULT_WIDTH = 800;
CoverFlow.DEFAULT_BG_COLOR = '#000000'; 
CoverFlow.IMAGE_SEPARATION = 50;
CoverFlow.RIGHT = 'right';
CoverFlow.LEFT = 'left';
CoverFlow.LABEL_CLASS = 'coverFlowLabel';
	
CoverFlow.prototype = {
    //Images array (it's a sort of transient var)
    images: [],
    //Items array {CoverFlowItem[]}
    coverFlowItems: [],
		
    remainingImages: 9999,
		
    element: null,
    labelElement: null,
    containerHeight: 0,
    containerWidth: 0,
		
    imageHeightRatio: 0.6,
    imageWidthRatio: 0.2,
    reflectionRatio: 0.6, // this causes:  imageTotalHeightRatio = imageHeightRatio + imageHeightRatio*reflectionRatio
    topRatio: 0.1,
    sideRatio: 0.4,
		
    perspectiveAngle: 20,
    imageZIndex: 1000,
    selectedImageZIndex: 9999,
    selectedItem: 0,
		
    moveQueue: [],
    animationWorking: false,
		
    init: function(el, userConfig){
		
        this.element = Dom.get(el);
        this.applyConfig(userConfig);
			
        if(userConfig.images)
            this.addImages(userConfig.images);
			
        this.attachEventListeners();
        this.createLabelElement();
    },
		
    applyConfig: function(config){
        this.containerHeight = config.height || CoverFlow.DEFAULT_HEIGHT;
        this.containerWidth = config.width || CoverFlow.DEFAULT_WIDTH;
        this.backgroundColor = config.bgColor || CoverFlow.DEFAULT_BG_COLOR;
			
        this.element.style.position = 'relative';
        this.element.style.height = this.containerHeight + 'px';
        this.element.style.width = this.containerWidth + 'px';
        this.element.style.background = this.backgroundColor;
        this.element.style.overflow = 'hidden';
    },
		
    addImages: function(images){
        this.images = [];
        this.remainingImages = images.length;

        for(var i=0; i < images.length; i++){
            var img = images[i];
            var image = new Image();
            image.id = Dom.generateId();
            image.index = i;
            image.onclick = img.onclick;
            image.label = img.label;
				
            //hide images
            image.style.visibility = 'hidden';
            image.style.display = 'none';
            //this is to maintain image order since image.onload will be called randomly
            this.element.appendChild(image);
            //a shortcut to not create another context to call onload
            var me = this;
            //				image.onload = function(){
            //					CoverFlow.preloadImage(me, this); // this = image
            //				};
            YAHOO.util.Event.on(image, 'load', this.preloadImage, image, this);
            image.src = img.src;
				
        };
			
    },
		
    /**
		 * @function preloadImage
		 * @param event
		 * @param image
		 * @return void
		 */
    preloadImage : function(e, image){
        this.images.push(image);
        this.checkAllImagesLoaded();
    },
		
    checkAllImagesLoaded: function(){
        this.remainingImages--;
        if(!this.remainingImages){
            this.setup();
        }
    },
		
    setup: function(){
        this.createCoverFlowItems();
        this.sortCoverFlowItems();
        this.initCoverFlow();
    },
		
    initCoverFlow: function(){
			
        for(var i=0; i < this.coverFlowItems.length; i++){
            var coverFlowItem = this.coverFlowItems[i];
				
            var angle = 0;
            var direction;

            if(i==0){
                coverFlowItem.setZIndex(this.selectedImageZIndex);
                coverFlowItem.setLeft(this.getCenter() - coverFlowItem.element.width/2);
                coverFlowItem.isSelected(true);
                this.selectedItem = 0;
                this.showLabel(coverFlowItem.getLabel());
            }else{
                angle = this.perspectiveAngle;
                direction = CoverFlow.LEFT;
                coverFlowItem.setZIndex(this.imageZIndex - i);
                coverFlowItem.setLeft( this.getRightStart()+ (i - 1)* CoverFlow.IMAGE_SEPARATION);
                coverFlowItem.isSelected(false);
            }
            coverFlowItem.setAngle(angle);
            coverFlowItem.drawInPerspective(direction);
        }
    },
		
    createLabelElement: function(){
        var label = document.createElement('div');
        label.id = Dom.generateId();
        label.style.position = 'absolute';
        label.style.top = this.getFooterOffset() + 'px';
        label.innerHTML = ' ';
        label.style.textAlign = 'center';
        label.style.width = this.containerWidth + 'px';
        label.style.zIndex = this.selectedImageZIndex + 10;
        label.className = CoverFlow.LABEL_CLASS;
        this.labelElement = this.element.appendChild(label);
    },
		
    showLabel: function(text){
        if(text)
            this.labelElement.innerHTML = text;
        else
            this.labelElement.innerHTML = '';
    },
		
    attachEventListeners: function(){
        new YAHOO.util.KeyListener(this.element, {
            keys:39
        },
{
            fn:this.selectNext,
            scope:this,
            correctScope:true
        } ).enable();

        new YAHOO.util.KeyListener(this.element, {
            keys:37
        },
{
            fn:this.selectPrevious,
            scope:this,
            correctScope:true
        } ).enable();
			

    },
		
    select: function(e,coverFlowItem){
        var distance = this.selectedItem - coverFlowItem.index;
        if(distance < 0){
            for(var i=0; i < -distance; i++)
                this.selectNext();
        }else{
            for(var i=0; i < distance; i++)
                this.selectPrevious();
        }
    },
		
		
    selectNext: function(){
        if(this.animationWorking){
            this.moveQueue.push('moveLeft');
            return;
        }
			
        var animateItems = [];
			
        for(var i=0; i < this.coverFlowItems.length; i++){
            var coverFlowItem = this.coverFlowItems[i];
            var isLast = (this.selectedItem == this.coverFlowItems.length -1);
            if(!isLast){
                var distance = i-this.selectedItem;

                if(distance == 0){// selected
                    coverFlowItem.setZIndex(this.imageZIndex);
                    coverFlowItem.isSelected(false);
                    animateItems.push({
                        item: coverFlowItem,
                        attribute:{
                            angle: {
                                start: 0,
                                end: this.perspectiveAngle
                                }
                            }
                    });

            coverFlowItem = this.coverFlowItems[++i];
            coverFlowItem.isSelected(true);
            this.showLabel(coverFlowItem.getLabel());
            animateItems.push({
                item: coverFlowItem,
                attribute:{
                    angle: {
                        start: this.perspectiveAngle,
                        end: 0
                    }
                }
            });
						
    }else{
        animateItems.push({
            item: coverFlowItem,
            attribute: {
                left: {
                    start:coverFlowItem.getLeft(),
                    end: coverFlowItem.getLeft() - CoverFlow.IMAGE_SEPARATION
                    }
                }
        });
}
}
}
			
var animation = new CoverFlowAnimation({
    direction: CoverFlow.LEFT,
    center: this.getCenter(),
    startLeftPos: this.getLeftStart(),
    startRightPos: this.getRightStart()
}, 
animateItems, 0.5);
	
animation.onStart.subscribe(this.handleAnimationWorking, this);
animation.onComplete.subscribe(this.handleQueuedMove, this);

animation.animate();

if(this.selectedItem + 1 < this.coverFlowItems.length)
    this.selectedItem++;
},

selectPrevious: function(){
    if(this.animationWorking){
        this.moveQueue.push('moveRight');
        return;
    }
			
    var animateItems = [];
			
    for(var i=0; i < this.coverFlowItems.length; i++){
        var coverFlowItem = this.coverFlowItems[i];
        var isFirst = (this.selectedItem == 0);
        var distance = i-this.selectedItem;
        if(!isFirst){
            if(distance == - 1){
                coverFlowItem.setZIndex(this.selectedImageZIndex);
                coverFlowItem.isSelected(true);
                this.showLabel(coverFlowItem.getLabel());
                animateItems.push({
                    item: coverFlowItem,
                    attribute: {
                        angle: {
                            start: this.perspectiveAngle,
                            end: 0
                        }
                    }
                });
						
        coverFlowItem = this.coverFlowItems[++i];
        coverFlowItem.isSelected(false);
        coverFlowItem.setZIndex(this.imageZIndex);
        animateItems.push({
            item: coverFlowItem,
            attribute: {
                angle: {
                    start: 0,
                    end: this.perspectiveAngle
                    }
                }
        });
}else{
    coverFlowItem.setZIndex(coverFlowItem.getZIndex() - 1);
    animateItems.push({
        item: coverFlowItem,
        attribute: {
            left: {
                start:coverFlowItem.getLeft(),
                end: coverFlowItem.getLeft() + CoverFlow.IMAGE_SEPARATION
                }
            }
    });
}
}
}
var animation = new CoverFlowAnimation({
    direction: CoverFlow.RIGHT,
    center: this.getCenter(),
    startLeftPos: this.getLeftStart(),
    startRightPos: this.getRightStart()
}, 
animateItems, 0.5);
			
animation.onStart.subscribe(this.handleAnimationWorking, this);
animation.onComplete.subscribe(this.handleQueuedMove, this);
			
animation.animate();

if(this.selectedItem > 0)
    this.selectedItem--;
},
		
handleAnimationWorking: function(a, b, cf){
    cf.animationWorking = true;
},
		
handleQueuedMove: function(msg, data, cf){
    cf.animationWorking = false;
			
    var next = cf.moveQueue.pop();
    if(next == 'moveLeft')
        cf.selectNext();
    if(next == 'moveRight')
        cf.selectPrevious();
},
		
getCenter: function(){
    return this.containerWidth / 2;
},
		
getRightStart: function() {
    return this.containerWidth - this.sideRatio * this.containerWidth;
},
		
getLeftStart: function() {
    return this.sideRatio * this.containerWidth;
},
		
sortCoverFlowItems: function(){
    function sortFunction(aCoverFlowItem, bCoverFlowItem){
        return aCoverFlowItem.index - bCoverFlowItem.index;
    }
			
    this.coverFlowItems.sort(sortFunction);
},
		
createCoverFlowItems: function(){
    this.coverFlowItems = [];
    for(var i=0; i<this.images.length; i++){
        var image = this.images[i];
        var coverFlowItem = new CoverFlowItem(image, {
            scaledWidth: this.scaleWidth(image),
            scaledHeight: this.scaleHeight(image),
            reflectionRatio: this.reflectionRatio,
            bgColor: this.backgroundColor,
            onclick: {
                fn: this.select,
                scope: this
            }
        });
        this.alignCenterHeight(coverFlowItem);
        this.coverFlowItems.push(coverFlowItem);
    };
    delete this.images;
},
		
alignCenterHeight: function(coverFlowItem){//review!!!!!
    coverFlowItem.element.style.position = 'absolute';
			
    var imageHeight = coverFlowItem.canvas.height / (1 + this.reflectionRatio);
    var top = this.getMaxImageHeight() - imageHeight;
    top += this.topRatio * this.containerHeight;
			
    coverFlowItem.setTop(top);
			
},
		
scaleHeight: function(image){
    var height = 0;
    if(image.height <= this.getMaxImageHeight()	&& image.width <= this.getMaxImageWidth()){
        height = image.height;
    }
    if(image.height > this.getMaxImageHeight()	&& image.width <= this.getMaxImageWidth()){
        height = ((image.height / this.getMaxImageHeight())) * image.height;
    }
    if(image.height <= this.getMaxImageHeight()	&& image.width > this.getMaxImageWidth()){
        height = ((image.width / this.getMaxImageWidth())) * image.height;
    }
    if(image.height > this.getMaxImageHeight()	&& image.width > this.getMaxImageWidth()){
        if(image.height > image.width)
            height = ((this.getMaxImageHeight() / image.height)) * image.height;
        else
            height = ((this.getMaxImageWidth() / image.width)) * image.height;
    }
    return height;
},

scaleWidth: function(image){
    var width = 0;
    if(image.height <= this.getMaxImageHeight()	&& image.width <= this.getMaxImageWidth()){
        width = image.width;
    }
    if(image.height > this.getMaxImageHeight()	&& image.width <= this.getMaxImageWidth()){
        width = ((image.height / this.getMaxImageHeight())) * image.width;
    }
    if(image.height <= this.getMaxImageHeight()	&& image.width > this.getMaxImageWidth()){
        width = ((image.width / this.getMaxImageWidth())) * image.width;
    }
    if(image.height > this.getMaxImageHeight()	&& image.width > this.getMaxImageWidth()){
        if(image.height > image.width)
            width = ((this.getMaxImageHeight() / image.height)) * image.width;
        else
            width = ((this.getMaxImageWidth() / image.width)) * image.width;
    }
    return width;
},
		
		
getMaxImageHeight: function(){
    return (this.containerHeight * this.imageHeightRatio);
},
		
getMaxImageWidth: function(){
    return (this.containerWidth * this.imageWidthRatio);
},
		
getTopOffset: function(){
    return this.containerHeight * this.topRatio;
},
		
getFooterOffset: function(){
    return this.containerHeight * (this.topRatio + this.imageHeightRatio);
}
};
	
	
/**
	 * @class CoverFlowItem 
	 * 
	 */
CoverFlowItem = function(image, config){
    if(image)
        this.init(image, config);
};
	
CoverFlowItem.prototype = {
    canvas: null,
    element: null,
    index: null,
    id: null,
    angle: 0,
    selected: false,
    onclickFn: null,
    selectedOnclickFn: null,
    label: null,
		
    onSelected: null,
		
    init: function(image, config){
        var scaledWidth = config.scaledWidth;
        var scaledHeight = config.scaledHeight;
        var reflectionRatio = config.reflectionRatio;
        var bgColor = config.bgColor;
			
        this.id = image.id;
        this.index = image.index;
        this.onclickFn = config.onclick;
        this.selectedOnclickFn = image.onclick;
        this.label = image.label;
        var parent = image.parentNode;
        this.canvas = this.createImageCanvas(image,scaledWidth,scaledHeight,reflectionRatio, bgColor);
        this.element = this.canvas.cloneNode(false);
        this.element.id = this.id;
        parent.replaceChild(this.element, image);
			
        this.onSelected = new YAHOO.util.CustomEvent('onSelected', this);
        this.onSelected.subscribe(this.handleOnclick);
			
    },
		
    getLabel: function(){
        return this.label;
    },
		
    handleOnclick: function(){
        YAHOO.util.Event.removeListener(this.element, 'click');
        if(!this.selected){
            YAHOO.util.Event.addListener(this.element, 'click', this.onclickFn.fn, this, this.onclickFn.scope);
        }else{
            if(this.selectedOnclickFn && this.selectedOnclickFn.fn)
                YAHOO.util.Event.addListener(this.element, 'click', this.selectedOnclickFn.fn, this, this.selectedOnclickFn.scope);
            else
                YAHOO.util.Event.addListener(this.element, 'click', this.selectedOnclickFn);
        }
    },
		
    isSelected: function(selected){
        this.selected = selected;
        this.onSelected.fire();
    },
		
    setAngle: function(angle){
        this.angle = angle;
    },
		
    getAngle: function(){
        return this.angle;
    },
		
    setTop: function(top){
        this.element.style.top = top + 'px';
    },
		
    setLeft: function(left){
        this.element.style.left = left + 'px';
    },
		
    getLeft: function(){
        var ret = this.element.style.left;
        return new Number(ret.replace("px", ""));
    },
		
    getZIndex: function(){
        return this.element.style.zIndex;
    },
		
    setZIndex: function(zIndex){
        this.element.style.zIndex = zIndex;
    },
		
    createImageCanvas: function(image, sWidth, sHeight, reflectionRatio, bgColor){

        var imageCanvas = document.createElement('canvas');
			
        if(imageCanvas.getContext){
				
            var scaledWidth = sWidth;
            var scaledHeight = sHeight;
            var reflectionHeight = scaledHeight * reflectionRatio;
				
            imageCanvas.height = scaledHeight + reflectionHeight;
            imageCanvas.width = scaledWidth;
				
            var ctx = imageCanvas.getContext('2d');
			
            ctx.clearRect(0, 0, imageCanvas.width, imageCanvas.height);
            ctx.globalCompositeOperation = 'source-over';
            ctx.fillStyle = 'rgba(0, 0, 0, 1)';
            ctx.fillRect(0, 0, imageCanvas.width, imageCanvas.height);
				
            //draw the reflection image
            ctx.save();
            ctx.translate(0, (2*scaledHeight));
            ctx.scale(1, -1);
            ctx.drawImage(image, 0, 0, scaledWidth, scaledHeight);
            ctx.restore();
            //create the gradient effect
            ctx.save();
            ctx.translate(0, scaledHeight);
            ctx.globalCompositeOperation = 'destination-out';
            var grad = ctx.createLinearGradient( 0, 0, 0, scaledHeight);
            grad.addColorStop(1, 'rgba(0, 0, 0, 1)');
            grad.addColorStop(0, 'rgba(0, 0, 0, 0.75)');
            ctx.fillStyle = grad;
            ctx.fillRect(0, 0, scaledWidth, scaledHeight);
            //apply the background color to the gradient
            ctx.globalCompositeOperation = 'destination-over';
            ctx.fillStyle = bgColor;
            '#000';
            ctx.globalAlpha = 0.8;
            ctx.fillRect(0, 0 , scaledWidth, scaledHeight);
            ctx.restore();
            //draw the image
            ctx.save();
            ctx.translate(0, 0);
            ctx.globalCompositeOperation = 'source-over';
            ctx.drawImage(image, 0, 0, scaledWidth, scaledHeight);
            ctx.restore();
				
            return imageCanvas;
        }
    },
		
    drawInPerspective: function(direction, frameSize){
        var canvas = this.element;
        var image = this.canvas;
        var angle = Math.ceil(this.angle);
        var ctx;
        var originalWidth = image.width;
        var originalHeight = image.height;
        var destinationWidth = destinationWidth || originalWidth; // for future use
        var destinationHeight = destinationHeight || originalHeight; // for future use
			
        var perspectiveCanvas = document.createElement('canvas');
        perspectiveCanvas.height = destinationHeight;
        perspectiveCanvas.width = destinationWidth;
        var perspectiveCtx = perspectiveCanvas.getContext('2d');

        var alpha = angle * Math.PI/180; // Math uses radian
			
        if(alpha > 0){ // if we have an angle greater than 0 then apply the perspective
            var right = (direction == CoverFlow.RIGHT);

            var initialX=0, finalX=0, initialY=0, finalY=0;

            frameSize = frameSize || 1;
            var xDes, yDes;
            var heightDes, widthDes;
            var perspectiveWidht = destinationWidth;
				
            var frameFactor = frameSize / originalWidth;
            var frames = Math.floor(originalWidth / frameSize);
	
            var widthSrc = frameSize ;
            var heightSrc = originalHeight;
				
            for(var i=0; i < frames; i++){
                var xSrc = (i) * frameSize;
                var ySrc = 0;
                var betaTan = 0;
                width = destinationWidth * (i) * frameFactor;
                horizon = destinationHeight / 2;
	
                if(right){
                    betaTan = horizon/((Math.tan(alpha)*horizon) + width);
                    xDes = (betaTan*width)/(Math.tan(alpha) + betaTan);
                    yDes = Math.tan(alpha) * xDes;
						
                    if(i == frames -1){
                        finalX=xDes;
                        finalY=yDes;
                    }
                }else{
                    betaTan = horizon/((Math.tan(alpha)*horizon) +(destinationWidth-width));
                    xDes = (Math.tan(alpha)*(destinationWidth) + (betaTan * width))/(Math.tan(alpha) + betaTan);
                    yDes = -Math.tan(alpha)*xDes + (Math.tan(alpha)*(destinationWidth));
						
                    if(i == 0){
                        initialX = xDes;
                        initialY = yDes;
                        finalX = destinationWidth;
                        finalY = 0;
                    }
                }
	
                heightDes = destinationHeight - (2*yDes);
                widthDes = heightDes / destinationHeight * destinationWidth;
					
                perspectiveCtx.drawImage(image, xSrc, ySrc, widthSrc, heightSrc, xDes, yDes, widthDes, heightDes);
			
            }

            perspectiveWidth = finalX - initialX;
            originalCanvasWidth = destinationWidth;
            canvas.width = perspectiveWidth;
				
            ctx = canvas.getContext('2d');
				
            //remove exceeded pixels
            ctx.beginPath();
            if(right){
                ctx.moveTo(0, 0);
                ctx.lineTo(finalX, finalY);
                ctx.lineTo(finalX, finalY + (destinationHeight - 2*finalY));
                ctx.lineTo(0, destinationHeight);
                ctx.lineTo(0,0);
            }else{
                var initialX1 = initialX - (originalCanvasWidth - perspectiveWidth);
                var finalX1 = finalX - (originalCanvasWidth - perspectiveWidth);
                ctx.moveTo(0, initialY);
                ctx.lineTo(finalX1, finalY);
                ctx.lineTo(finalX1, destinationHeight);
                ctx.lineTo(initialX1, initialY + (destinationHeight - 2*initialY));
                ctx.lineTo(0, initialY);
            }
            ctx.closePath();
            ctx.clip();
				
            ctx.drawImage(perspectiveCanvas, initialX, 0, perspectiveWidth, destinationHeight, 0, 0, perspectiveWidth, destinationHeight);
			
        }else{
				
            canvas.width = perspectiveCanvas.width;
            canvas.height = perspectiveCanvas.height;
            perspectiveCtx.drawImage(image, 0, 0, originalWidth, originalHeight, 0, 0, destinationWidth, destinationHeight);
            ctx = canvas.getContext('2d');
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.drawImage(perspectiveCanvas, 0, 0);
        }
    }
		
};
	
/**
	 * @class CoverFlowAnimation
	 * @requires YAHOO.util.AnimMgr
	 */
CoverFlowAnimation = function(config, animationItems, duration){
    this.init(config, animationItems, duration);
};	
	
CoverFlowAnimation.prototype = {
    direction: null,
		
    center: null,
		
    startLeftPos: null,
		
    startRightPos: null,
		
    animationItems: null,

    method : YAHOO.util.Easing.easeNone,
		
    animated: false,
		
    startTime: null,
		
    actualFrames : 0,
		
    useSeconds : true, // default to seconds
		
    currentFrame : 0,
		
    totalFrames : YAHOO.util.AnimMgr.fps,
			
    init: function(config, animationItems, duration){
        this.direction = config.direction;
        this.center = config.center;
        this.startLeftPos = config.startLeftPos;
        this.startRightPos = config.startRightPos;
        this.animationItems = animationItems;
        this.duration = duration || 1;
        this.registerEvents();
    },
		
    registerEvents: function(){
        /**
	         * Custom event that fires after onStart, useful in subclassing
	         * @private
	         */    
        this._onStart = new YAHOO.util.CustomEvent('_start', this, true);

        /**
	         * Custom event that fires when animation begins
	         * Listen via subscribe method (e.g. myAnim.onStart.subscribe(someFunction)
	         * @event onStart
	         */    
        this.onStart = new YAHOO.util.CustomEvent('start', this);
	        
        /**
	         * Custom event that fires between each frame
	         * Listen via subscribe method (e.g. myAnim.onTween.subscribe(someFunction)
	         * @event onTween
	         */
        this.onTween = new YAHOO.util.CustomEvent('tween', this);
	        
        /**
	         * Custom event that fires after onTween
	         * @private
	         */
        this._onTween = new YAHOO.util.CustomEvent('_tween', this, true);
	        
        /**
	         * Custom event that fires when animation ends
	         * Listen via subscribe method (e.g. myAnim.onComplete.subscribe(someFunction)
	         * @event onComplete
	         */
        this.onComplete = new YAHOO.util.CustomEvent('complete', this);
        /**
	         * Custom event that fires after onComplete
	         * @private
	         */
        this._onComplete = new YAHOO.util.CustomEvent('_complete', this, true);

        this._onStart.subscribe(this.doOnStart);
        this._onTween.subscribe(this.doOnTween);
        this._onComplete.subscribe(this.doOnComplete);
			
    },
        
    isAnimated : function() {
        return this.animated;
    },
        
    getStartTime : function() {
        return this.startTime;
    },
        
    doMethod: function(start, end) {
        return this.method(this.currentFrame, start, end - start, this.totalFrames);
    },
        
    animate : function() {
        if ( this.isAnimated() ) {
            return false;
        }
            
        this.currentFrame = 0;
            
        this.totalFrames = ( this.useSeconds ) ? Math.ceil(YAHOO.util.AnimMgr.fps * this.duration) : this.duration;
    
        if (this.duration === 0 && this.useSeconds) { // jump to last frame if zero second duration
            this.totalFrames = 1;
        }
        YAHOO.util.AnimMgr.registerElement(this);
        return true;
    },
          
    stop : function(finish) {
        if (!this.isAnimated()) { // nothing to stop
            return false;
        }

        if (finish) {
            this.currentFrame = this.totalFrames;
            this._onTween.fire();
        }
        YAHOO.util.AnimMgr.stop(this);
    },
        
    doOnStart : function() {
        this.onStart.fire();
            
        this.runtimeItems = [];
        for (var i=0; i<this.animationItems.length; i++) {
            this.setRuntimeItem(this.animationItems[i]);
        }
            
        this.animated = true;
        this.actualFrames = 0;
        this.startTime = new Date();
    },
        
    doOnTween : function() {
        var data = {
            duration: new Date() - this.getStartTime(),
            currentFrame: this.currentFrame
        };
            
        data.toString = function() {
            return (
                'duration: ' + data.duration +
                ', currentFrame: ' + data.currentFrame
                );
        };
            
        this.onTween.fire(data);
            
        this.actualFrames += 1;

        var runtimeItems = this.runtimeItems;
            
        for (var i=0; i < runtimeItems.length; i++) {
            this.setItemAttributes(runtimeItems[i]);
        }
            
    },
        
    doOnComplete : function() {
        var actual_duration = (new Date() - this.getStartTime()) / 1000 ;
            
        var data = {
            duration: actual_duration,
            frames: this.actualFrames,
            fps: this.actualFrames / actual_duration
        };
            
        data.toString = function() {
            return (
                'duration: ' + data.duration +
                ', frames: ' + data.frames +
                ', fps: ' + data.fps
                );
        };
            
        this.animated = false;
        this.actualFrames = 0;
        this.onComplete.fire(data);
    },
        
    setRuntimeItem: function(item){
        var runtimeItem = {};
        runtimeItem.item = item.item;
        runtimeItem.attribute = {};
        for(var attr in item.attribute){
            runtimeItem.attribute[attr] = item.attribute[attr];
            if(attr == 'angle'){
                if(item.attribute[attr].start - item.attribute[attr].end > 0){
                    runtimeItem.attribute[attr].perspectiveDirection = this.direction;
                    runtimeItem.attribute[attr].center = true;
                }else{
                    runtimeItem.attribute[attr].perspectiveDirection = this.direction == CoverFlow.RIGHT ? CoverFlow.LEFT : CoverFlow.RIGHT;
                    runtimeItem.attribute[attr].center = false;
                }
            }
        }
        this.runtimeItems.push(runtimeItem);
    },
        
    setItemAttributes: function(item){
        	
        for(var attr in item.attribute){
        
            var value = Math.ceil(this.doMethod(item.attribute[attr].start, item.attribute[attr].end));
        		
            if(attr == 'angle'){
                item.item.setAngle(value);
                var frameSize = Math.ceil(this.doMethod(3, 1));
                item.item.drawInPerspective(item.attribute[attr].perspectiveDirection, frameSize);
                var left;
                if(item.attribute[attr].center){
                    left = this.doMethod(item.item.getLeft(), this.center - item.item.element.width/2);
                }else{
                    if(this.direction == CoverFlow.LEFT)
                        left = this.doMethod(item.item.getLeft(), this.startLeftPos - item.item.element.width);
                    else
                        left = this.doMethod(item.item.getLeft(), this.startRightPos);
                }
                item.item.setLeft(Math.ceil(left));
        		
            }else{
                item.item.setLeft(value);
            }
        }
    }
};
	
//});