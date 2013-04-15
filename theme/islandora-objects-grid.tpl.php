<?php

/**
 * @file
 * Render a bunch of objects in a list or grid view.
 */
?>
<div class="islandora-objects-grid clearfix">
 <?php foreach($objects as $object): ?>
   <div class="islandora-objects-grid-item clearfix">
     <dl class="islandora-object <?php print $object['class']; ?>">
       <dt class="islandora-object-thumb"><?php print $object['thumb']; ?></dt>
       <dd class="islandora-object-caption"><?php print $object['link']; ?></dd>
     </dl>
   </div>
 <?php endforeach; ?>
</div>
