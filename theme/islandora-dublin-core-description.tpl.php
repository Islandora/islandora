<?php
/**
 * @file
 * This is the template file for the Dublin Core metadata description.
 *
 * Available variables:
 * - $islandora_object: The Islandora object rendered in this template file
 * $dc_array: The DC datastream object values as a sanitized array. This
 * includes label, value and class name.
 *
 * @see template_preprocess_islandora_dublin_core_description()
 * @see theme_islandora_dublin_core_description()
 */
?>
<div class="islandora-metadata-sidebar">
  <?php if (!empty($dc_array['dc:description']['value'])): ?>
    <h2><?php print $dc_array['dc:description']['label']; ?></h2>
    <p property="description"><?php print $dc_array['dc:description']['value']; ?></p>
  <?php endif; ?>
</div>
