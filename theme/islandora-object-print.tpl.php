<?php

/**
 * @file
 * The default view to print objects.
 *
 */
?>
<div>
  <div>
    <?php print $islandora_content; ?>
  </div>
  <fieldset class="islandora-basic-image-metadata islandora">
  <legend><span class="fieldset-legend"><?php print t('Details'); ?></span></legend>
    <div class="fieldset-wrapper">
      <dl class="islandora-inline-metadata islandora-basic-image-fields">
        <?php $row_field = 0; ?>
        <?php foreach( $dc_array as $key => $value): ?>
          <dt class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['label']; ?>
          </dt>
          <dd class="<?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
            <?php print $value['value']; ?>
          </dd>
          <?php $row_field++; ?>
        <?php endforeach; ?>
      </dl>
    </div>
  </fieldset>
</div>
