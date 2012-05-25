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
<div class="islandora-basic-collection clearfix">
  <?php foreach($associated_objects_array as $key => $value): ?>
    <dl class="islandora-basic-collection-object">
        <dt class="<?php print $value['class']; ?>"><?php print $value['thumb_link']; ?></dt>
        <dd class="<?php print $value['class']; ?>"><?php print $value['title_link']; ?></dd>
    </dl>
  <?php endforeach; ?>
</div>
