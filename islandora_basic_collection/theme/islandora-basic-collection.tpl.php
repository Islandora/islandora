<?php

/*
 * islandora-basic-collection.tpl.php
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
<?php drupal_set_title($islandora_object->label); ?>

<div class="islandora islandora-basic-collection">
    <?php $row_field = 0; ?>
    <?php foreach($associated_objects_array as $associated_object): ?>
      <div class="islandora-basic-collection-object islandora-basic-collection-list-item clearfix"> 
        <dl class="<?php print $associated_object['class']; ?>">
            <dt>
              <?php print $associated_object['thumb_link']; ?>
            </dt>
            <dd class="collection-value <?php print $associated_object['dc_array']['dc:title']['class']; ?> <?php print $row_field == 0 ? ' first' : ''; ?>">
              <strong><?php print $associated_object['title_link']; ?></strong>
            </dd>
            <?php if ($associated_object['dc_array']['dc:description']['value']): ?>
              <dd class="collection-value <?php print $associated_object['dc_array']['dc:description']['class']; ?>">
                <?php print $associated_object['dc_array']['dc:description']['value']; ?>
              </dd>
            <?php endif; ?>
        </dl>
      </div>
    <?php $row_field++; ?>
    <?php endforeach; ?>
</div>