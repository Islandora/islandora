<?php

/**
 * @file
 * Render a bunch of objects in a list or grid view.
 */
?>
<div class="islandora-objects clearfix">
  <span class="islandora-objects-display-switch">
    <?php 
    print theme('links', array(
                           'links' => $display_links,
                           'attributes' => array('class' => array('links', 'inline')),
                         )
    );
    ?>
  </span>
  <?php print $pager; ?>
  <?php print $content; ?>
  <?php print $pager; ?>
</div>
