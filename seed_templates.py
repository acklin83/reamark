#!/usr/bin/env python3
"""
Manually seed default email templates into the Mixnote database.
Run this from the mixnote project root: python seed_templates.py
"""

import sys
import os

# Add backend to path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'backend'))

from backend.app.database import SessionLocal
from backend.app.models import EmailTemplate

def seed_templates():
    db = SessionLocal()
    try:
        # Check existing templates
        count = db.query(EmailTemplate).count()
        print(f"Current templates in database: {count}")

        if count >= 2:
            print("Database already has both templates. Exiting.")
            return

        # German template
        tpl_de = EmailTemplate(
            name="Standard-Benachrichtigung (Deutsch)",
            subject="Neuer {{ 'Reply' if is_reply else 'Kommentar' }} – {{ project_title }} / {{ song_title }}",
            body_html="""<div style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; color: #333;">
  <p style="font-size: 16px; font-weight: bold; color: #6366f1; margin: 0 0 4px 0;">{{ project_title }}</p>
  <p style="color: #888; margin-top: 0; font-size: 14px;">{{ song_title }} &middot; v{{ version_number }} {{ version_label }}</p>
  <hr style="border: none; border-top: 1px solid #e5e7eb; margin: 16px 0;">
  {% if is_reply %}
  <p style="color: #888; font-size: 14px;">Antwort auf Kommentar von <strong>{{ parent_comment_author }}</strong>:</p>
  <div style="border-left: 3px solid #6366f1; padding-left: 12px; margin: 8px 0; color: #666; font-size: 14px;">
    {{ parent_comment_text }}
  </div>
  {% endif %}
  <div style="background: #f9fafb; padding: 16px; border-radius: 8px; margin: 16px 0;">
    <p style="margin: 0 0 8px 0; font-size: 14px;"><strong>{{ author_name }}</strong> <span style="color: #888; font-size: 13px;">@ {{ timecode }}</span></p>
    <p style="margin: 0; font-size: 15px;">{{ comment_text }}</p>
  </div>
  <p style="font-size: 14px;"><a href="{{ share_url }}" style="color: #6366f1; text-decoration: none;">In Mixnote &ouml;ffnen &rarr;</a></p>
</div>""",
        )

        # English template
        tpl_en = EmailTemplate(
            name="Standard Notification (English)",
            subject="New {{ 'Reply' if is_reply else 'Comment' }} – {{ project_title }} / {{ song_title }}",
            body_html="""<div style="font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif; max-width: 600px; margin: 0 auto; padding: 20px; color: #333;">
  <p style="font-size: 16px; font-weight: bold; color: #6366f1; margin: 0 0 4px 0;">{{ project_title }}</p>
  <p style="color: #888; margin-top: 0; font-size: 14px;">{{ song_title }} &middot; v{{ version_number }} {{ version_label }}</p>
  <hr style="border: none; border-top: 1px solid #e5e7eb; margin: 16px 0;">
  {% if is_reply %}
  <p style="color: #888; font-size: 14px;">Reply to comment by <strong>{{ parent_comment_author }}</strong>:</p>
  <div style="border-left: 3px solid #6366f1; padding-left: 12px; margin: 8px 0; color: #666; font-size: 14px;">
    {{ parent_comment_text }}
  </div>
  {% endif %}
  <div style="background: #f9fafb; padding: 16px; border-radius: 8px; margin: 16px 0;">
    <p style="margin: 0 0 8px 0; font-size: 14px;"><strong>{{ author_name }}</strong> <span style="color: #888; font-size: 13px;">@ {{ timecode }}</span></p>
    <p style="margin: 0; font-size: 15px;">{{ comment_text }}</p>
  </div>
  <p style="font-size: 14px;"><a href="{{ share_url }}" style="color: #6366f1; text-decoration: none;">Open in Mixnote &rarr;</a></p>
</div>""",
        )

        # Add templates
        if count == 0:
            print("Adding both German and English templates...")
            db.add(tpl_de)
            db.add(tpl_en)
        elif count == 1:
            existing = db.query(EmailTemplate).first()
            if existing and "Deutsch" in existing.name:
                print("Adding English template (German already exists)...")
                db.add(tpl_en)
            else:
                print("Adding German template (English already exists)...")
                db.add(tpl_de)

        db.commit()

        # Show results
        templates = db.query(EmailTemplate).all()
        print(f"\n✓ Successfully seeded templates. Total count: {len(templates)}")
        for tpl in templates:
            print(f"  - {tpl.name} (ID: {tpl.id})")

    except Exception as e:
        print(f"Error: {e}")
        db.rollback()
    finally:
        db.close()

if __name__ == "__main__":
    seed_templates()
