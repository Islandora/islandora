


YAHOO.util.Event.onDOMReady(function(){

    var images = [
    {
        src: 'images/ardillitaMac.jpg',
        label: 'Ardileta!',
        onclick: function(){
            alert('image1');
        }
    },
{
    src: 'http://farm2.static.flickr.com/1380/1426855399_b4b8eccbdb.jpg?v=0'
},
{
    src: 'http://farm1.static.flickr.com/69/213130158_0d1aa23576_d.jpg'
},
{
    src: 'http://farm1.static.flickr.com/69/213130158_0d1aa23576_d.jpg'
},
{
    src: 'images/msn2.jpg',
    label: 'My Mac'
},
{
    src: 'images/msn2.jpg',
    label: 'My Mac again...'
}
	              
];
var myCoverFlow = new YAHOO.ext.CoverFlow('coverFlowTest', {
    height: 200,
    width: 600,
    images: images
});

function moveLeft(e, coverFlow){
    coverFlow.selectNext();
}
function moveRight(e, coverFlow){
    coverFlow.selectPrevious();
}
var myMoveLeftBtn = new YAHOO.widget.Button('moveLeftButton', {
    onclick: {
        fn: moveLeft,
        obj: myCoverFlow
    }
});
var myMoveRightBtn = new YAHOO.widget.Button('moveRightButton', {
    onclick: {
        fn: moveRight,
        obj: myCoverFlow
    }
});

	
var otherImages = [
{
    src: 'images/ardillitaMac.jpg',
    label: 'Ardileta!',
    onclick: function(){
        alert('image1');
    }
},
{
    src: 'images/msn2.jpg',
    label: 'My Mac'
},
{
    src: 'images/msn2.jpg',
    label: 'My Mac again...'
}
	              
];	
var anotherCoverFlow = new YAHOO.ext.CoverFlow('anotherCoverFlowTest', {
    height: 150,
    width: 500,
    images: otherImages,
    bgColor: '#C0C0C0'
});



});