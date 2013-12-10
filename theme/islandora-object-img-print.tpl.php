<?php

/**
 * @file
* The default view to theme an image of an object.
* 
* This view is passed into 'islandora-object-print' theme file
* and is rendred as an image. Allows for seperate theming of image
* and metadata.
*
*/

?>
<?php if (isset($islandora_content)): ?>
  <div>
    <?php print $islandora_content; ?>
  </div>
<?php endif; ?>
