<?php
/*
 * islandora-basic-image--islandora-27.tpl.php
 * 
 *
 * 
 * This file is part of Islandora.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with the program.  If not, see <http ://www.gnu.org/licenses/>.
 */
?>
<?php drupal_set_title(""); ?>

<div class="islandora-basic-image-object">
  <div class="islandora-basic-image-content clearfix">
    <?php print $islandora_medium_img; ?> 
  </div>
  <div class="islandora-basic-image-sidebar">
    <h1 class="title"><?php print $islandora_object_label; ?></h1>
    <h3><?php print $dc_array['dc:description']['label']; ?></h3>
    <p><?php print $dc_array['dc:description']['value']; ?></p>
  </div>
  <div class="islandora-basic-image-metadata">
    <h4>Details</h4>
    <dl class="islandora-basic-image-fields">
      <?php $row_field = 0; ?>
      <?php foreach($dc_array as $key => $value): ?>
        <dt class="solr-label <?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
          <?php print $value['label']; ?>
        </dt>
        <?php if ($key == 'PID'): ?>
          <?php $value['value'] = l($value['value'], 'fedora/repository/' . htmlspecialchars($value['value'], ENT_QUOTES, 'utf-8')); ?>
        <?php endif; ?>
        <dd class="solr-value <?php print $value['class']; ?><?php print $row_field == 0 ? ' first' : ''; ?>">
          <?php print $value['value']; ?>
        </dd>
        <?php $row_field++; ?>
      <?php endforeach; ?>
    </dl>
  </div>
</div>
